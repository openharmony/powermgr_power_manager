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

#ifndef POWERMGR_POWER_MANAGER_SHUTDOWN_PRIORITY_H
#define POWERMGR_POWER_MANAGER_SHUTDOWN_PRIORITY_H

namespace OHOS {
namespace PowerMgr {
/**
 * Shutdown or reboot priority.
 *
 * Callbacks are triggered from highest to lowest priority.
 *
 * NOTE: HIGHEST and LOWEST are reserved for system-critical internal users
 * only. Normal users MUST use LOW / DEFAULT / HIGH. The reserved priorities
 * exist so that platform-level bookkeeping (e.g. last-resort persistence,
 * final teardown) can be ordered strictly before/after all user callbacks
 * without overloading the existing three buckets.
 */
enum class ShutdownPriority : uint32_t {
    /** Low priority */
    LOW = 0,
    /** Default priority */
    DEFAULT,
    /** High priority */
    HIGH,
    /**
     * Highest priority. Runs before HIGH.
     * Reserved for internal/system-critical use only. NOT for normal users.
     */
    HIGHEST,
    /**
     * Lowest priority. Runs after LOW.
     * Reserved for internal/system-critical use only. NOT for normal users.
     */
    LOWEST,
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_MANAGER_SHUTDOWN_PRIORITY_H
