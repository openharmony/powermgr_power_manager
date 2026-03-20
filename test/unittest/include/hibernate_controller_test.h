/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef HIBERNATE_CONTROLLER_TEST_H_
#define HIBERNATE_CONTROLLER_TEST_H_

#ifdef POWER_GTEST
#define private    public
#define protected  public
#endif

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>
#include "power_log.h"

#include "hibernate/hibernate_controller.h"
#include "sync_hibernate_callback_stub.h"

namespace OHOS {
namespace PowerMgr {

class HibernateControllerTest : public testing::Test {
public:
    std::unique_ptr<HibernateController> hibernateController_;

    void SetUp();
    void TearDown();
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
};

class PowerSyncHibernateTestCallback : public IRemoteStub<ISyncHibernateCallback> {
public:
    PowerSyncHibernateTestCallback() {};
    virtual ~PowerSyncHibernateTestCallback() {};
    virtual void OnSyncHibernate() override;
    virtual void OnSyncWakeup(bool hibernateResult = false) override;
};
} // PowerMgr
} // OHOS
#endif //HIBERNATE_CONTROLLER_TEST_H_