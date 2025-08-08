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

/* This files contains faultlog fuzzer test modules. */

#define FUZZ_PROJECT_NAME "refreshactivity_fuzzer"

#include "power_fuzzer.h"
#include "ipower_mgr.h"

using namespace OHOS::PowerMgr;

namespace {
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    PowerFuzzerTest g_serviceTest;
    uint32_t code = static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REFRESH_ACTIVITY_IPC);
    g_serviceTest.TestPowerServiceStub(code, data, size);
    code = static_cast<uint32_t>(IPowerMgrIpcCode::COMMAND_REFRESH_ACTIVITY_IPC_IN_LONG_IN_INT_IN_STRING_OUT_INT);
    g_serviceTest.TestPowerServiceStub(code, data, size);
    return 0;
}
