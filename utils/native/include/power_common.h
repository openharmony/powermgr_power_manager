/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_POWER_COMMON_H
#define POWERMGR_POWER_COMMON_H

#include <cstdint>
#include <type_traits>

#include "power_log.h"
#include "power_mgr_errors.h"

namespace OHOS {
namespace PowerMgr {
#define RETURN_IF_WITH_RET(cond, retval) if (cond) {return (retval);}
#define RETURN_IF(cond) if (cond) {return;}
#define RETURN_IF_WITH_LOG(cond, loginfo)                                                   \
    do {                                                                                \
        if (cond) {                                                                     \
            POWER_HILOGE(COMP_FWK, #loginfo);                                           \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define READ_PARCEL_NO_RET(parcel, type, out)                                           \
    do {                                                                                \
        if (!(parcel).Read##type(out)) {                                                \
            POWER_HILOGE(COMP_FWK, "read "#out" failed");                          \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define WRITE_PARCEL_NO_RET(parcel, type, data)                                         \
    do {                                                                                \
        if (!(parcel).Write##type(data)) {                                              \
            POWER_HILOGE(COMP_FWK, "write "#data" failed");                        \
            return;                                                                     \
        }                                                                               \
    } while (0)                                                                         \

#define READ_PARCEL_WITH_RET(parcel, type, out, retval)                                \
    do {                                                                               \
        if (!(parcel).Read##type(out)) {                                               \
            POWER_HILOGE(COMP_FWK, "read "#out" failed");                         \
            return (retval);                                                           \
        }                                                                              \
    } while (0)                                                                        \

#define WRITE_PARCEL_WITH_RET(parcel, type, data, retval)                              \
    do {                                                                               \
        if (!(parcel).Write##type(data)) {                                             \
            POWER_HILOGE(COMP_FWK, "write data failed");                       \
            return (retval);                                                           \
        }                                                                              \
    } while (0)

template<typename E>
constexpr auto ToUnderlying(E e) noexcept
{
    return static_cast<std::underlying_type_t<E>>(e);
}
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_COMMON_H
