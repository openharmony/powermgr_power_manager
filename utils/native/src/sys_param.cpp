/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "sys_param.h"

#include <unordered_map>

#include <parameter.h>

using namespace std;

namespace OHOS {
namespace PowerMgr {
const DeviceType SysParam::DEVICE_TYPE = InitDeviceType();

DeviceType SysParam::InitDeviceType()
{
    static const unordered_map<string, DeviceType> DEVICE_TYPE_MAP {
        { "default", DeviceType::DEVICE_PHONE },
        { "phone", DeviceType::DEVICE_PHONE },
        { "watch", DeviceType::DEVICE_WATCH },
        { "tv", DeviceType::DEVICE_TV },
        { "car", DeviceType::DEVICE_CAR },
        { "tablet", DeviceType::DEVICE_TABLET },
    };
    static string type = GetDeviceType();
    auto it = DEVICE_TYPE_MAP.find(type);
    return (it != DEVICE_TYPE_MAP.end()) ? it->second : DeviceType::DEVICE_UNKNOWN;
}
} // namespace PowerMgr
} // namespace OHOS
