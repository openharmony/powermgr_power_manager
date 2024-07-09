/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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

#ifndef POWER_TIME_H
#define POWER_TIME_H

namespace OHOS {
namespace PowerMgr {
using namespace std::chrono;
constexpr int32_t MS_TO_S = 1000;
constexpr int32_t NS_TO_MS = 1000000;

static int64_t GetCurrentRealTimeMs()
{
    struct timespec ts = { 0, 0};
    if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
        return 0;
    }
    return (ts.tv_sec * MS_TO_S + ts.tv_nsec / NS_TO_MS);
}

}
}
#endif