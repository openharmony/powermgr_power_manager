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

#ifndef POWER_HDF_CLIENT_H
#define POWER_HDF_CLIENT_H

#include <stdlib.h>
#include "ipc_object_proxy.h"
#include "power_hdf_info.h"

namespace OHOS {
namespace PowerMgr {
class PowerHdfClient {
public:
    PowerHdfClient() = default;
    ~PowerHdfClient() = default;
    ErrCode Suspend();
    ErrCode ReadWakeCount(std::string& count);
    ErrCode WriteWakeCount(const std::string& count);
    ErrCode WakeLock(const std::string& name);
    ErrCode WakeUnlock(const std::string& name);
    ErrCode Dump(std::string& info);
private:
    ErrCode Dispatch(uint32_t cmd, MessageParcel &data, MessageParcel &reply);
    sptr<IRemoteObject> GetService();
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWER_HDF_CLIENT_H