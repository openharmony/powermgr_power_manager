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

#ifndef POWER_KEY_OPTION_TEST_H
#define POWER_KEY_OPTION_TEST_H

#include <gtest/gtest.h>

#ifdef POWERMGR_GTEST
#define private   public
#define protected public
#endif

namespace OHOS {
namespace PowerMgr {
class PowerKeyOptionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

enum class SubscriberState : int32_t {
    FAILURE = -1,
    SUCCESS = 0,
    RETRY_SUCCESS = 1
};

class RequestContext;
class ServiceState {
public:
    ServiceState() = default;
    virtual ~ServiceState() = default;
    virtual SubscriberState Handle(RequestContext& context) = 0;
private:
    static inline int32_t deathRetryCount_ = 0;
};

class RequestContext {
public:
    explicit RequestContext(const std::shared_ptr<ServiceState> state) : state_(state) {}
    ~RequestContext() = default;
    std::shared_ptr<ServiceState> state_ {nullptr};
    SubscriberState HandleRequest();
};

class AliveServiceState : public ServiceState {
public:
    AliveServiceState() = default;
    virtual ~AliveServiceState() = default;
    SubscriberState Handle(RequestContext& context) override;
};

class RestartingServiceState : public ServiceState {
public:
    RestartingServiceState() = default;
    virtual ~RestartingServiceState() = default;
    SubscriberState Handle(RequestContext& context) override;
};

class DeadServiceState : public ServiceState {
public:
    DeadServiceState() = default;
    virtual ~DeadServiceState() = default;
    SubscriberState Handle(RequestContext& context) override;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWER_KEY_OPTION_TEST_H