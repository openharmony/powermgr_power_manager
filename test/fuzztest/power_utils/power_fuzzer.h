/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

/* This files contains faultlog config modules. */

#ifndef POWER_FUZZER_TEST_H
#define POWER_FUZZER_TEST_H

#include "power_mgr_service.h"

class PowerFuzzerTest {
public:
    PowerFuzzerTest();
    ~PowerFuzzerTest();
    void TestPowerServiceStub(const uint32_t code, const uint8_t* data, size_t size);

private:
    OHOS::sptr<OHOS::PowerMgr::PowerMgrService> service_ {nullptr};
};

#endif
