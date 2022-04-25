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

import {AsyncCallback} from './basic';

/**
 * Provides interfaces to manage power.
 *
 * @syscap SystemCapability.PowerManager.PowerManager.Core
 * @since 7
 */
declare namespace power {
  /**
   * Shuts down the system.
   *
   * <p>This method requires the ohos.permission.REBOOT permission.
   *
   * @param reason Indicates the shutdown reason.
   * @permission ohos.permission.REBOOT
   * @systemapi
   * @since 7
   */
  function shutdownDevice(reason: string): void;

  /**
   * Restarts the system.
   *
   * <p>This method requires the ohos.permission.REBOOT permission.
   *
   * @param reason Indicates the restart reason. For example, "updater" indicates entering the updater mode
   * after the restart. If the parameter is not specified, the system enters the normal mode after the restart.
   * @permission ohos.permission.REBOOT
   * @since 7
   */
  function rebootDevice(reason: string): void;

  /**
   * Checks whether the screen of a device is on or off.
   *
   * @return Returns true if the screen is on; returns false otherwise.
   * @since 7
   */
  function isScreenOn(callback: AsyncCallback<boolean>): void;
  function isScreenOn(): Promise<boolean>;

  /**
   * Try to wakeup the device and let screen on.
   *
   * @param detail Indicates the detail information who request wakeup.
   * @systemapi
   * @since 9
   */
   function wakeupDevice(detail: string): void;

   /**
    * Try to suspend the device and let screen off.
    *
    * @systemapi
    * @since 9
    */
   function suspendDevice(): void;

  /**
   * Get the power mode of the device.
   *
   * @return Returns the power mode {@link DevicePowerMode} of current device .
   * @permission ohos.permission.POWER_OPTIMIZATION
   * @since 9
   */
   function getPowerMode(callback: AsyncCallback<DevicePowerMode>): void;
   function getPowerMode(): Promise<DevicePowerMode>;
 
   /**
    * To set the power mode of current device.
    *
    * @param mode Indicates power mode {@link DevicePowerMode} to set.
    * @permission ohos.permission.POWER_OPTIMIZATION
    * @systemapi
    * @since 9
    */
   function setPowerMode(mode: DevicePowerMode, callback: AsyncCallback<void>): void;
   function setPowerMode(mode: DevicePowerMode): Promise<void>;
 
   /**
    * Indicates the power mode of a device.
    *
    * @syscap SystemCapability.PowerManager.PowerManager.Core
    * @since 9
    */
   export enum DevicePowerMode {
   /**
    * Performance power mode
    * @since 9
    */
   MODE_PERFORMANCE,
   /**
    * Normal power mode
    * @since 9
    */
   MODE_NORMAL,
   /**
    * Power save mode
    * @since 9
    */
   MODE_POWER_SAVE,
   /**
    * Extreme power save mode
    * @since 9
    */
   MODE_EXTREME_POWER_SAVE
   }
}
export default power;

