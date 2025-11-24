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

#include "power_mgr_service_ipc_adapter_test.h"
#include "ipower_mgr.h"
#include "iremote_object.h"
#include "power_log.h"
#include "power_mgr_async_reply.h"
#include "power_mgr_async_reply_stub.h"
#include "power_mgr_service.h"
#include "power_mgr_service_ipc_adapter.h"
#include "power_mgr_stub.h"
#include "sp_singleton.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;

class TestPowerMgrServiceAdapter : public PowerMgrServiceAdapter {
public:
    PowerErrors RebootDevice(const std::string& reason)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors RebootDeviceForDeprecated(const std::string& reason, bool force)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors ShutDownDevice(const std::string& reason)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors SetSuspendTag(const std::string& tag)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors SuspendDevice(
        int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed, const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors WakeupDevice(
        int64_t callTimeMs, WakeupDeviceType reason, const std::string& details, const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    void WakeupDeviceAsync(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details) {}
    bool RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
    {
        return true;
    }
    PowerErrors OverrideScreenOffTime(int64_t timeout, const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors RestoreScreenOffTime(const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    PowerState GetState()
    {
        return PowerState::AWAKE;
    }
    bool IsScreenOn(bool needPrintLog = true)
    {
        return true;
    }
    bool IsFoldScreenOn()
    {
        return true;
    }
    bool IsCollaborationScreenOn()
    {
        return true;
    }
    bool IsForceSleeping()
    {
        return true;
    }
    PowerErrors ForceSuspendDevice(int64_t callTimeMs, const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors Hibernate(bool clearMemory, const std::string& reason = "", const std::string& apiVersion = "-1")
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo)
    {
        return PowerErrors::ERR_OK;
    }
    bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "")
    {
        return true;
    }
    bool IsRunningLockTypeSupported(RunningLockType type)
    {
        return true;
    }
    bool UpdateWorkSource(const sptr<IRemoteObject>& remoteObj, const std::vector<int32_t>& workSources)
    {
        return true;
    }
    PowerErrors Lock(const sptr<IRemoteObject>& remoteObj, int32_t timeOutMs = -1)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors UnLock(const sptr<IRemoteObject>& remoteObj, const std::string& name = "")
    {
        return PowerErrors::ERR_OK;
    }
    bool QueryRunningLockLists(std::map<std::string, RunningLockInfo>& runningLockLists)
    {
        return true;
    }
    bool IsUsed(const sptr<IRemoteObject>& remoteObj)
    {
        return true;
    }
    bool ProxyRunningLock(bool isProxied, pid_t pid, pid_t uid)
    {
        return true;
    }
    bool ProxyRunningLocks(bool isProxied, const std::vector<std::pair<pid_t, pid_t>>& processInfos)
    {
        return true;
    }
    bool ResetRunningLocks()
    {
        return true;
    }
    bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback, bool isSync = true)
    {
        return true;
    }
    bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
    {
        return true;
    }

    bool RegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback, SleepPriority priority)
    {
        return true;
    }
    bool UnRegisterSyncSleepCallback(const sptr<ISyncSleepCallback>& callback)
    {
        return true;
    }
    bool RegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
    {
        return true;
    }
    bool UnRegisterSyncHibernateCallback(const sptr<ISyncHibernateCallback>& callback)
    {
        return true;
    }
    bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
    {
        return true;
    }
    bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
    {
        return true;
    }

    bool RegisterScreenStateCallback(int32_t remainTime, const sptr<IScreenOffPreCallback>& callback)
    {
        return true;
    }
    bool UnRegisterScreenStateCallback(const sptr<IScreenOffPreCallback>& callback)
    {
        return true;
    }

    bool RegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
    {
        return true;
    }
    bool UnRegisterRunningLockCallback(const sptr<IPowerRunninglockCallback>& callback)
    {
        return true;
    }
    bool SetDisplaySuspend(bool enable)
    {
        return true;
    }
    PowerErrors SetDeviceMode(const PowerMode& mode)
    {
        return PowerErrors::ERR_OK;
    }
    PowerMode GetDeviceMode()
    {
        return PowerMode::NORMAL_MODE;
    }
    std::string ShellDump(const std::vector<std::string>& args, uint32_t argc)
    {
        return "";
    }
    PowerErrors IsStandby(bool& isStandby)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors ForceRebootDevice(const std::string& reason)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors LockScreenAfterTimingOutWithAppid(
        pid_t appid, bool lockScreen, const sptr<IRemoteObject>& token)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors SetPowerKeyFilteringStrategy(PowerKeyFilteringStrategy strategy)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors SetForceTimingOut(bool enabled, const sptr<IRemoteObject>& token)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors LockScreenAfterTimingOut(
        bool enabledLockScreen, bool checkLock, bool sendScreenOffEvent, const sptr<IRemoteObject>& token, pid_t appid)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors IsRunningLockEnabled(const RunningLockType type, bool& result)
    {
        return PowerErrors::ERR_OK;
    }
    PowerErrors RefreshActivity(int64_t callTimeMs, UserActivityType type, const std::string& refreshReason)
    {
        return PowerErrors::ERR_OK;
    }
    void RegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback, ShutdownPriority priority) {}
    void UnRegisterShutdownCallback(const sptr<ITakeOverShutdownCallback>& callback) {}

    void RegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback, ShutdownPriority priority) {}
    void UnRegisterShutdownCallback(const sptr<IAsyncShutdownCallback>& callback) {}
    void RegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback, ShutdownPriority priority) {}
    void UnRegisterShutdownCallback(const sptr<ISyncShutdownCallback>& callback) {}
    bool RegisterSuspendTakeoverCallback(
        const sptr<ITakeOverSuspendCallback>& callback, TakeOverSuspendPriority priority)
    {
        return true;
    }
    bool UnRegisterSuspendTakeoverCallback(const sptr<ITakeOverSuspendCallback>& callback)
    {
        return true;
    }
};

class TestTakeOverSuspendCallback : public ITakeOverSuspendCallback {
public:
    TestTakeOverSuspendCallback() = default;
    virtual ~TestTakeOverSuspendCallback() = default;

    bool OnTakeOverSuspend(SuspendDeviceType type) override
    {
        return false;
    }
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

void PowerMgrServiceIpcAdapterTest::SetUpTestCase(void)
{
}
void PowerMgrServiceIpcAdapterTest::TearDownTestCase(void) {}
void PowerMgrServiceIpcAdapterTest::SetUp() {}
void PowerMgrServiceIpcAdapterTest::TearDown() {}
sptr<PowerMgrService> g_pmsTest = nullptr;

namespace {
/**
 * @tc.name: PowerMgrServiceIpcAdapter001
 * @tc.desc: test PowerMgrServiceIpcAdapter.RegisterSuspendTakeoverCallbackIpc
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter001, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter001 function start!");
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    TakeOverSuspendPriority priority = TakeOverSuspendPriority::HIGH;
    int32_t result = adapter->RegisterSuspendTakeoverCallbackIpc(callback, static_cast<int32_t>(priority));
    result = adapter->RegisterSuspendTakeoverCallbackIpc(callback, 10);
    EXPECT_EQ(result, -1);
    result = adapter->RegisterSuspendTakeoverCallbackIpc(nullptr, static_cast<int32_t>(priority));
    EXPECT_EQ(result, -1);
    sptr<TestTakeOverSuspendCallback> callback2 = new TestTakeOverSuspendCallback();
    result = adapter->RegisterSuspendTakeoverCallbackIpc(callback2, static_cast<int32_t>(-1));
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter001 function end!");
}

/**
 * @tc.name: PowerMgrServiceIpcAdapter002
 * @tc.desc: test PowerMgrServiceIpcAdapter.UnRegisterSuspendTakeoverCallback
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter002, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter002 function start!");
    sptr<TestTakeOverSuspendCallback> callback = new TestTakeOverSuspendCallback();
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    adapter->UnRegisterSuspendTakeoverCallbackIpc(callback);
    int32_t result = adapter->UnRegisterSuspendTakeoverCallbackIpc(nullptr);
    EXPECT_EQ(result, -1);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter002 function end!");
}

/**
 * @tc.name: PowerMgrServiceIpcAdapter003
 * @tc.desc: test PowerMgrServiceIpcAdapter.ForceSuspendDeviceIpc
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter003, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter003 function start!");
    std::string apiVersion = "-1";
    constexpr int64_t param = 0;
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    int32_t result = adapter->ForceSuspendDeviceIpc(param, apiVersion, powerProxy);
    EXPECT_NE(result, -1);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter003 function end!");
}

/**
 * @tc.name: PowerMgrServiceIpcAdapter004
 * @tc.desc: test PowerMgrServiceIpcAdapter.ForceSuspendDeviceIpc
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter004, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter004 function start!");
    std::string apiVersion = "-1";
    constexpr int64_t param = 0;
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    int32_t result = adapter->ForceSuspendDeviceIpc(param, apiVersion, nullptr);
    EXPECT_EQ(result, -1);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter004 function end!");
}

/**
 * @tc.name: PowerMgrServiceIpcAdapter005
 * @tc.desc: test PowerMgrServiceIpcAdapter.HibernateIpc
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter005, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter005 function start!");
    std::string apiVersion = "-1";
    bool clearMemory = true;
    std::string reason = "";
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    sptr<PowerMgrStubAsync> asyncCallback = new PowerMgrStubAsync();
    sptr<IPowerMgrAsync> powerProxy = iface_cast<IPowerMgrAsync>(asyncCallback);
    int32_t result = adapter->HibernateIpc(clearMemory, reason, apiVersion, powerProxy);
    EXPECT_NE(result, -1);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter005 function end!");
}

/**
 * @tc.name: PowerMgrServiceIpcAdapter006
 * @tc.desc: test PowerMgrServiceIpcAdapter.HibernateIpc
 * @tc.type: FUNC
 */
HWTEST_F(PowerMgrServiceIpcAdapterTest, PowerMgrServiceIpcAdapter006, TestSize.Level2) {
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter006 function start!");
    std::string apiVersion = "-1";
    bool clearMemory = true;
    std::string reason = "";
    auto adapter = DelayedSpSingleton<TestPowerMgrServiceAdapter>::GetInstance();
    int32_t result = adapter->HibernateIpc(clearMemory, reason, apiVersion, nullptr);
    EXPECT_EQ(result, -1);
    POWER_HILOGI(LABEL_TEST, "PowerMgrServiceIpcAdapter006 function end!");
}
}