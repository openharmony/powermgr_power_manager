/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <client_lifecycle.h>
#include <gtest/gtest.h>
#include <mock_lock_action.h>
#include <mock_power_action.h>
#include <mock_state_action.h>
#include <permission.h>
#include <power_mgr_client.h>
#include <power_mgr_proxy.h>
#include <power_mgr_service.h>
#include <shutdown/shutdown_client.h>
#include <shutdown/sync_shutdown_callback_stub.h>

#include <condition_variable>
#include <mutex>

#define SET_REBOOT     _IOW(BOOT_DETECTOR_IOCTL_BASE, 109, int)
#define SET_SHUT_STAGE _IOW(BOOT_DETECTOR_IOCTL_BASE, 106, int)

using namespace testing;
using namespace ext;
using namespace OHOS;
using namespace PowerMgr;

namespace {
unsigned int g_retCount = 0;
std::vector<int> g_retVec {};
bool g_boolRet = true;
constexpr const char* PATH = "/dev/bbox";
pid_t g_testingThreadId = 0;
bool g_isTesting = false;

bool IsCallFromTesting()
{
    return g_isTesting && gettid() == g_testingThreadId;
}
} // namespace

// redefinitions of C outer interfaces
extern "C" {
void fdsan_exchange_owner_tag(int fd, uint64_t expected_tag, uint64_t new_tag)
{
    static auto realFdsanExchange =
        reinterpret_cast<void (*)(int, uint64_t, uint64_t)>(dlsym(RTLD_NEXT, "fdsan_exchange_owner_tag"));
    // In this test (for now), the fake "open" will never return an fd > 0.
    // Remove the last condition if it is not the case.
    if (realFdsanExchange && (new_tag != DOMAIN_FEATURE_SHUTDOWN || !IsCallFromTesting() || fd > 0)) {
        realFdsanExchange(fd, expected_tag, new_tag);
    }
}

int fdsan_close_with_tag(int fd, uint64_t tag)
{
    static auto realFdsanClose = reinterpret_cast<int (*)(int, uint64_t)>(dlsym(RTLD_NEXT, "fdsan_close_with_tag"));
    if (tag != DOMAIN_FEATURE_SHUTDOWN || !IsCallFromTesting() || fd > 0) {
        return realFdsanClose ? realFdsanClose(fd, tag) : close(fd);
    }
    return 0;
}

// dlsym does not depend on open so it should be safe to call it inside my interceptor
int open(const char* path, int flag, ...)
{
    static auto realOpen = reinterpret_cast<int (*)(const char*, int, ...)>(dlsym(RTLD_NEXT, "open"));
    if (!IsCallFromTesting() || std::string {path} != PATH) {
        mode_t mode = 0;
        if ((flag & O_CREAT) || (flag & O_TMPFILE) == O_TMPFILE) {
            va_list args;
            va_start(args, flag);
            mode = va_arg(args, mode_t);
            va_end(args);
            return realOpen ? realOpen(path, flag, mode) : openat(AT_FDCWD, path, flag, mode);
        }
        return realOpen ? realOpen(path, flag) : openat(AT_FDCWD, path, flag);
    }

    POWER_HILOGI(LABEL_TEST, "mock open called");
    if (g_retCount < g_retVec.size()) {
        return g_retVec[g_retCount++];
    }
    return 0;
}

int ioctl(int fd, int cmd, ...)
{
    static auto realIoctl = reinterpret_cast<int (*)(int, int, ...)>(dlsym(RTLD_NEXT, "ioctl"));
    if (realIoctl && (!IsCallFromTesting() || (cmd != SET_REBOOT && cmd != SET_SHUT_STAGE))) {
        void* param;
        va_list args;
        va_start(args, cmd);
        param = va_arg(args, void*);
        va_end(args);
        return realIoctl(fd, cmd, param);
    }

    POWER_HILOGI(LABEL_TEST, "mock ioctl called");
    if (g_retCount < g_retVec.size()) {
        return g_retVec[g_retCount++];
    }
    return 0;
}
} // extern "C"

namespace OHOS::PowerMgr {
class GeneralInterfacesTest : public Test {
public:
    class MockProxy : public PowerMgrProxy {
    public:
        using PowerMgrProxy::PowerMgrProxy;
        MOCK_METHOD2(ForceRebootDeviceIpc, int32_t(const std::string&, int32_t&));
    };

    void SetProxyMockState(bool enable)
    {
        proxyMockEnabled = enable;
    }

    void SetUp(void)
    {
        // will be passed to smart pointers in EnableMock
        if (!stateActionMock || !shutdownStateActionMock || !powerActionMock || !lockActionMock) {
            stateActionMock = new NiceMock<MockStateAction>;
            shutdownStateActionMock = new NiceMock<MockStateAction>;
            powerActionMock = new NiceMock<MockPowerAction>;
            lockActionMock = new NiceMock<MockLockAction>;
            stub_->EnableMock(stateActionMock, shutdownStateActionMock, powerActionMock, lockActionMock);
        }
        g_boolRet = true; // controls Permission, refactor it later if I had time
    }

    void TearDown(void)
    {
        constexpr int maxTick = 100;
        constexpr int tickTimeUs = 1000 * 50;
        // try to wait for the detached thread to complete
        for (int tick = 0; tick < maxTick; tick++) {
            if (!stub_->GetShutdownController()->IsShuttingDown()) {
                break;
            }
            usleep(tickTimeUs);
        }

        // try to release the mock objects just before end of the current testcase
        if (!stub_->GetShutdownController()->IsShuttingDown()) {
            stub_->EnableMock(nullptr, nullptr, nullptr, nullptr);
            // EnableMock for PowerStateMachine ignores nullptr, reset it manually
            auto& stateAction =
                const_cast<std::shared_ptr<IDeviceStateAction>&>(stub_->GetPowerStateMachine()->GetStateAction());
            stateAction.reset();
            stateActionMock = nullptr;
            shutdownStateActionMock = nullptr;
            powerActionMock = nullptr;
            lockActionMock = nullptr;
        }
    }

    static void SetUpTestCase(void)
    {
        stub_ = DelayedSpSingleton<PowerMgrService>::GetInstance();
        stub_->OnStart();
        mockProxy_ = sptr<NiceMock<MockProxy>>::MakeSptr(stub_);
        g_testingThreadId = gettid();
    }

    static void TearDownTestCase(void) {}

    // data members
    NiceMock<MockStateAction>* stateActionMock {nullptr};
    NiceMock<MockStateAction>* shutdownStateActionMock {nullptr};
    NiceMock<MockPowerAction>* powerActionMock {nullptr};
    NiceMock<MockLockAction>* lockActionMock {nullptr};

    static inline bool proxyMockEnabled = false;
    static inline sptr<PowerMgrService> stub_ {nullptr};
    static inline sptr<NiceMock<MockProxy>> mockProxy_ {nullptr};
};

bool PowerMgr::Permission::IsSystem()
{
    return g_boolRet;
}

bool PowerMgr::Permission::IsPermissionGranted(const std::string&)
{
    return true;
}

sptr<IPowerMgr> PowerMgrClient::GetPowerMgrProxy()
{
    if (GeneralInterfacesTest::proxyMockEnabled) {
        return GeneralInterfacesTest::mockProxy_;
    } else {
        return GeneralInterfacesTest::stub_;
    }
}

sptr<IPowerMgr> ClientLifeCycle::GetProxy()
{
    if (GeneralInterfacesTest::proxyMockEnabled) {
        return GeneralInterfacesTest::mockProxy_;
    } else {
        return GeneralInterfacesTest::stub_;
    }
}

// test cases below
namespace {
// cover branches of client side call
HWTEST_F(GeneralInterfacesTest, ForceRebootDeviceTest001, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest001 function start!");

    SetProxyMockState(true);

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    ASSERT_TRUE(powerActionMock != nullptr);
    constexpr int retVecSize = 4;
    for (unsigned int cases = 0; cases < (1 << retVecSize); cases++) {
        g_retCount = 0;
        g_retVec.clear();
        for (int index = 0; index < retVecSize; index++) {
            g_retVec.emplace_back((cases & (1 << index)) == 0 ? -1 : 0);
        }
        EXPECT_CALL(*mockProxy_, ForceRebootDeviceIpc("Some Reason", _));
        g_isTesting = true;
        powerMgrClient.ForceRebootDevice("Some Reason");
        g_isTesting = false;
    }

    SetProxyMockState(false);
    // mock object released, EXPECT_CALL is checked here. Consider moving construction/destruction to setup/teardown
    mockProxy_ = nullptr;

    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest001 function end!");
}

std::mutex g_cvMutex;
std::condition_variable g_cv;
bool g_notified = false;

HWTEST_F(GeneralInterfacesTest, ForceRebootDeviceTest002, TestSize.Level0)
{
    // cover some branches which are unlikely to be accessed
    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest002 function start!");
    auto& powerMgrClient = PowerMgrClient::GetInstance();

    g_isTesting = true;

    // Make permission check to fail
    g_boolRet = false;
    ASSERT_TRUE(powerActionMock != nullptr);
    EXPECT_CALL(*powerActionMock, Reboot(_)).Times(0);
    powerMgrClient.ForceRebootDevice("Some Reason");
    g_boolRet = true;

    // Non-force and reason != "test_case"
    EXPECT_CALL(*powerActionMock, Reboot("Some Reason"));
    powerMgrClient.RebootDevice("Some Reason");

    // Non-force and reason != "test_case"
    EXPECT_CALL(*powerActionMock, Reboot("test_case"));
    powerMgrClient.RebootDevice("test_case");

    // Reboot blocks so that the next call will be skipped(since started_ == true), for next non-force reboot
    EXPECT_CALL(*powerActionMock, Reboot("Some Reason")).WillOnce(Invoke([]() {
        POWER_HILOGI(LABEL_TEST, "blocking reboot action called");
        std::unique_lock lock(g_cvMutex);
        g_cv.wait(lock, []() { return g_notified; });
        g_notified = false;
        POWER_HILOGI(LABEL_TEST, " reboot action unblocked");
    }));
    // first call, reboot blocks
    powerMgrClient.RebootDevice("Some Reason");
    // second call, return early
    powerMgrClient.RebootDevice("Some Reason");
    g_isTesting = false;
    // unblocks the blocking reboot
    {
        std::lock_guard lock(g_cvMutex);
        g_notified = true;
        g_cv.notify_all();
    }
    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest002 function end!");
}

class SyncShutdownCallback : public SyncShutdownCallbackStub {
public:
    ~SyncShutdownCallback() override = default;
    void OnSyncShutdownOrReboot(bool) override
    {
        std::unique_lock lock(g_cvMutex);
        g_cv.wait(lock, []() { return g_notified; });
    }
};

// test async process when calling ForceReboot
// callback blocks, reboot unblocks
HWTEST_F(GeneralInterfacesTest, ForceRebootDeviceTest003, TestSize.Level0)
{
    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest003 function start!");
    sptr<ISyncShutdownCallback> callback = sptr<SyncShutdownCallback>::MakeSptr();
    ShutdownClient::GetInstance().RegisterShutdownCallback(callback);
    ASSERT_TRUE(powerActionMock != nullptr);
    EXPECT_CALL(*powerActionMock, Reboot("Some Reason")).WillOnce(Invoke([]() {
        POWER_HILOGI(LABEL_TEST, "unblocking reboot action called");
        std::unique_lock lock(g_cvMutex);
        g_notified = true;
        g_cv.notify_all();
    }));

    auto& powerMgrClient = PowerMgrClient::GetInstance();
    g_isTesting = true;
    powerMgrClient.ForceRebootDevice("Some Reason");
    g_isTesting = false;
    {
        std::unique_lock lock(g_cvMutex);
        EXPECT_FALSE(g_notified);
        // only be unblocked after Reboot is called
        g_cv.wait(lock, []() { return g_notified; });
    }
    POWER_HILOGI(LABEL_TEST, "ForceRebootDeviceTest003 function end!");
}
} // namespace
} // namespace OHOS::PowerMgr
