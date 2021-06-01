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

#ifndef POWERMGR_SYS_PARAM_H
#define POWERMGR_SYS_PARAM_H

#include <string>

namespace OHOS {
namespace PowerMgr {
enum class DeviceType : int32_t {
    DEVICE_UNKNOWN = -1,
    DEVICE_PHONE = 0,
    DEVICE_TV,
    DEVICE_SPEAKER,
    DEVICE_GLASSES,
    DEVICE_WATCH,
    DEVICE_CAR,
    DEVICE_EARPHONE,
    DEVICE_PC,
    DEVICE_TABLET,
};

class SysParam {
public:
    static inline DeviceType GetDeviceTypeParam()
    {
        return DEVICE_TYPE;
    }

    static inline bool IsDeviceType(DeviceType type)
    {
        return type == DEVICE_TYPE;
    }

private:
    static DeviceType InitDeviceType();

    static const DeviceType DEVICE_TYPE;
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SYS_PARAM_H
