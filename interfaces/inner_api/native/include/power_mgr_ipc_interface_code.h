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

#ifndef POWER_MGR_IPC_INTERFACE_DODE_H
#define POWER_MGR_IPC_INTERFACE_DODE_H

/* SAID: 3301 */
namespace OHOS {
namespace PowerMgr {
enum class PowerMgrInterfaceCode {
    CREATE_RUNNINGLOCK = 0,
    RELEASE_RUNNINGLOCK,
    IS_RUNNINGLOCK_TYPE_SUPPORTED,
    RUNNINGLOCK_LOCK,
    RUNNINGLOCK_UNLOCK,
    RUNNINGLOCK_QUERY,
    RUNNINGLOCK_ISUSED,
    PROXY_RUNNINGLOCK,
    PROXY_RUNNINGLOCKS,
    RESET_RUNNINGLOCKS,
    WAKEUP_DEVICE,
    SUSPEND_DEVICE,
    REFRESH_ACTIVITY,
    OVERRIDE_DISPLAY_OFF_TIME,
    RESTORE_DISPLAY_OFF_TIME,
    GET_STATE,
    IS_SCREEN_ON,
    FORCE_DEVICE_SUSPEND,
    REBOOT_DEVICE,
    REBOOT_DEVICE_FOR_DEPRECATED,
    SHUTDOWN_DEVICE,
    REG_POWER_STATE_CALLBACK,
    UNREG_POWER_STATE_CALLBACK,
    REG_POWER_MODE_CALLBACK,
    UNREG_POWER_MODE_CALLBACK,
    SET_DISPLAY_SUSPEND,
    SETMODE_DEVICE,
    GETMODE_DEVICE,
    SHELL_DUMP,
    IS_STANDBY,
    REG_SYNC_SLEEP_CALLBACK,
    UNREG_SYNC_SLEEP_CALLBACK,
    SET_FORCE_TIMING_OUT,
    LOCK_SCREEN_AFTER_TIMING_OUT,
    REG_RUNNINGLOCK_CALLBACK,
    UNREG_RUNNINGLOCK_CALLBACK,
    HIBERNATE,
    REG_SCREEN_OFF_PRE_CALLBACK,
    UNREG_SCREEN_OFF_PRE_CALLBACK,
};
} // space PowerMgr
} // namespace OHOS

#endif // POWER_MGR_IPC_INTERFACE_DODE_H