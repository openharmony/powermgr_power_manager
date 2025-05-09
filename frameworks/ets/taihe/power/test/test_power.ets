/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

import _power from "@ohos.power"

export namespace testPower {
    export function testwakeup() {
        _power.wakeup('testwakeup');
    }

    export function testsuspend() {
        _power.suspend();
    }

    export function testsuspendWiteParam() {
        _power.suspend(true);
    }

    export function testshutdown() {
        _power.shutdown("testshutdown");
    }

    export function testreboot() {
        _power.reboot("testreboot");
    }

    export function testisActive() {
        _power.isActive();
    }

    export function testgetPowerMode() {
        _power.getPowerMode();
    }

    export function testsetPowerModePromise() {
        _power.setPowerMode(DevicePowerMode.MODE_PERFORMANCE);
    }

    export function testsetPowerModeCallback() {
        _power.setPowerMode(DevicePowerMode.MODE_PERFORMANCE, (): void => {
            console.info("testsetPowerModeCallback call");
        });
    }

    export function testisStandby() {
        _power.isStandby();
    }

    export function testhibernate() {
        _power.hibernate();
    }

    export function testsetScreenOffTime() {
        _power.setScreenOffTime(3000);
    }
}
