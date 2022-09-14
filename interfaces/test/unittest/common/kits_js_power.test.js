/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

import power from '@ohos.power';
import { describe, it, expect } from 'deccjsunit/index'

describe('PowerMgrPowerUnitTest', function () {
    /**
     * @tc.number PowerMgrPowerUnitTest001
     * @tc.name Screen off
     * @tc.desc Screen off
     */
    it('power_suspend', 0, async function (done) {
        power.isScreenOn()
            .then(screenOn => {
                console.info('power_suspend01: The current screenOn is ' + screenOn);
                if (!screenOn) {
                    power.wakeupDevice("power_suspend");
                }
            })
            .catch(error => {
                console.log('power_suspend error: ' + error);
                expect().assertFail();
                done();
            })
        power.suspendDevice();
        power.isScreenOn().then(screenOn => {
            console.info('power_suspend02: The current screenOn is ' + screenOn);
            expect(screenOn).assertFalse();
            console.info('power_suspend success');
        }).catch(error => {
            console.log('power_suspend error: ' + error);
            expect().assertFail();
            done();
        })
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest002
     * @tc.name Screen on
     * @tc.desc Screen on
     */
    it('power_wake_up_success', 0, async function (done) {
        power.isScreenOn()
            .then(screenOn => {
                console.info('The current screenOn is ' + screenOn);
                if (screenOn) {
                    power.suspendDevice();
                }
            })
            .catch(error => {
                console.log('power_wake_up_success error: ' + error);
                expect().assertFail();
                done();
            })
        power.wakeupDevice("power_wake_up_success");
        power.isScreenOn().then(screenOn => {
            console.info('The current screenOn is ' + screenOn);
            expect(screenOn).assertTrue();
            console.info('power_wake_up_success success');
        }).catch(error => {
            console.log('power_wake_up_success error: ' + error);
            expect().assertFail();
            done();
        })
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest003
     * @tc.name Screen on
     * @tc.desc Screen on
     */
    it('power_wake_up_fail', 0, async function (done) {
        power.isScreenOn()
            .then(screenOn => {
                console.info('power_wake_up_fail01: The current screenOn is ' + screenOn);
                if (screenOn) {
                    power.suspendDevice();
                }
            })
            .catch(error => {
                console.log('power_wake_up_fail error: ' + error);
                expect().assertFail();
                done();
            })
        try {
            power.wakeupDevice(1)
        } catch (error) {
            console.log('power_wake_up_fail wakeupDevice error: ' + error);
        }
        power.isScreenOn().then(screenOn => {
            console.info('power_wake_up_fail02: The current screenOn is ' + screenOn);
            expect(screenOn).assertFalse();
            console.info('power_wake_up_fail success');
        }).catch(error => {
            console.log('power_wake_up_fail error: ' + error);
            expect(error === "Wrong argument type. string expected.").assertTrue();
            done();
        })
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest004
     * @tc.name power_is_screen_on_promise_test
     * @tc.desc isScreenOn
     */
    it('power_is_screen_on_promise_test', 0, async function (done) {
        power.wakeupDevice("power_is_screen_on_promise_test");
        power.isScreenOn()
            .then(screenOn => {
                console.info('power_is_screen_on_promise_test screenOn is ' + screenOn);
                expect(screenOn).assertTrue();
                console.info('power_is_screen_on_promise_test success');
            })
            .catch(error => {
                console.log('power_is_screen_on_promise_test error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest005
     * @tc.name power_is_screen_on_callback_test
     * @tc.desc isScreenOn
     */
    it('power_is_screen_on_callback_test', 0, async function (done) {
        power.wakeupDevice("power_is_screen_on_callback_test");
        power.isScreenOn((error, screenOn) => {
            if (typeof error === "undefined") {
                console.info('power_is_screen_on_callback_test screenOn is ' + screenOn);
                expect(screenOn).assertTrue();
                console.info('power_is_screen_on_callback_test success');
            } else {
                console.log('power_is_screen_on_callback_test: ' + error);
                expect().assertFail();
                done();
            }
        })
        done();
    })

    // New interface
    /**
     * @tc.number PowerMgrPowerUnitTest006
     * @tc.name get_set_mode_promise
     * @tc.desc Gets and sets the power mode
     */
    it('get_set_mode_promise', 0, async function (done) {
        let currentMode = power.getPowerMode();
        console.info('get_set_mode_promise currentMode:' + currentMode);
        expect(currentMode >= power.DevicePowerMode.MODE_NORMAL &&
            currentMode <= power.DevicePowerMode.MODE_EXTREME_POWER_SAVE).assertTrue();
        try {
            let isExec = false;
            power.setPowerMode(power.DevicePowerMode.MODE_POWER_SAVE)
                .then((error) => {
                    isExec = true;
                    console.info('get_set_mode_promise error:' + (typeof error));
                    expect(typeof error === "undefined").assertTrue();
                    let mode = power.getPowerMode();
                    console.info('get_set_mode_promise mode:' + mode);
                    expect(mode === power.DevicePowerMode.MODE_POWER_SAVE).assertTrue();
                }).finally(() => {
                    expect(isExec).assertTrue();
                })
        } catch (e) {
            console.info('get_set_mode_promise error:' + e);
            expect().assertFail();
        }
        power.setPowerMode(currentMode);
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest007
     * @tc.name set_mode_promise_invalid
     * @tc.desc Set the wrong power mode
     */
    it('set_mode_promise_invalid', 0, async function (done) {
        try {
            // invalid mode
            power.setPowerMode('').then(() => {
                console.info('set_mode_promise_invalid_string failed');
                expect().assertFail();
            })
        } catch (e) {
            console.info('set_mode_promise_invalid code:' + e.code + 'msg:' + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }

        try {
            // Number of invalid parameters
            power.setPowerMode('', '', 0).then(() => {
                console.info('set_mode_promise_invalid_number failed');
                expect().assertFail();
            })
        } catch (e) {
            console.info('set_mode_promise_invalid code:' + e.code + 'msg:' + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest008
     * @tc.name get_set_mode_callback
     * @tc.desc Gets and sets the power mode
     */
    it('get_set_mode_callback', 0, async function (done) {
        let currentMode = power.getPowerMode();
        console.info('get_set_mode_callback currentMode:' + currentMode);
        expect(currentMode >= power.DevicePowerMode.MODE_NORMAL &&
            currentMode <= power.DevicePowerMode.MODE_EXTREME_POWER_SAVE).assertTrue();
        try {
            power.setPowerMode(power.DevicePowerMode.MODE_PERFORMANCE, (error) => {
                console.info('get_set_mode_callback error:' + (typeof error));
                expect(typeof error === "undefined").assertTrue();
                let mode = power.getPowerMode();
                console.info('get_set_mode_callback mode:' + mode);
                expect(mode === power.DevicePowerMode.MODE_PERFORMANCE).assertTrue();
            })
        } catch (e) {
            console.info('get_set_mode_callback error:' + e);
            expect().assertFail();
        }
        power.setPowerMode(currentMode);
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest009
     * @tc.name set_mode_callback_invalid
     * @tc.desc Set the wrong power mode
     */
    it('set_mode_callback_invalid', 0, async function (done) {
        try {
            // invalid mode
            power.setPowerMode('', (error) => {
                expect().assertFail();
            })
        } catch (e) {
            console.info('set_mode_callback_invalid code:' + e.code + 'msg:' + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }

        try {
            // invalid callback
            power.setPowerMode(power.DevicePowerMode.MODE_PERFORMANCE, '')
        } catch (e) {
            console.info('set_mode_callback_invalid code:' + e.code + 'msg:' + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest010
     * @tc.name power_device_power_mode_MODE_NORMAL
     * @tc.desc Get device power mode MODE_NORMAL
     */
    it('power_device_power_mode_normal', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_NORMAL;
        console.info('MODE_NORMAL = ' + devicePowerMode);
        expect(devicePowerMode === 600).assertTrue();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest011
     * @tc.name power_device_power_mode_MODE_POWER_SAVE
     * @tc.desc Get device power mode MODE_POWER_SAVE
     */
    it('power_device_power_mode_power_save', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_POWER_SAVE;
        console.info('MODE_POWER_SAVE = ' + devicePowerMode);
        expect(devicePowerMode === 601).assertTrue();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest012
     * @tc.name power_device_power_mode_MODE_PERFORMANCE
     * @tc.desc Get device power mode MODE_PERFORMANCE
     */
    it('power_device_power_mode_performance', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_PERFORMANCE;
        console.info('MODE_PERFORMANCE = ' + devicePowerMode);
        expect(devicePowerMode === 602).assertTrue();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest013
     * @tc.name power_device_power_mode_MODE_EXTREME_POWER_SAVE
     * @tc.desc Get device power mode MODE_EXTREME_POWER_SAVE
     */
    it('power_device_power_mode_extreme_power_save', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_EXTREME_POWER_SAVE;
        console.info('MODE_EXTREME_POWER_SAVE = ' + devicePowerMode);
        expect(devicePowerMode === 603).assertTrue();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest014
     * @tc.name shutdown_invalid
     * @tc.desc Invalid shutdown parameters
     */
    it('shutdown_invalid', 0, function () {
        try {
            power.shutdown(0);
        } catch (e) {
            console.info('shutdown_invalid code:' + e.code + 'msg:' + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
    })

    /**
     * @tc.number PowerMgrPowerUnitTest015
     * @tc.name power_shutdown
     * @tc.desc shutdown
     */
    it('power_shutdown', 0, function () {
        // Shutdown tests are not performed by default
        if (false) {
            try {
                power.shutdown('PowerMgrPowerUnitTest');
            } catch (e) {
                console.info('power_shutdown error:' + e);
                expect().assertFail();
            }
        }
    })

    /**
     * @tc.number PowerMgrPowerUnitTest016
     * @tc.name power_reboot
     * @tc.desc reboot
     */
    it('power_reboot', 0, function () {
        // Reboot tests are not performed by default
        if (false) {
            try {
                power.reboot('PowerMgrPowerUnitTest');
            } catch (e) {
                console.info('power_reboot error:' + e);
                expect().assertFail();
            }
        }
    })

    /**
     * @tc.number PowerMgrPowerUnitTest017
     * @tc.name wakeup_suspend_is_active
     * @tc.desc On and Off Screen and get the current screen on and off
     */
    it('wakeup_suspend_is_active', 0, function () {
        try {
            power.suspend();
            power.wakeup('wakeup_suspend_is_active');
            let on = power.isActive();
            console.info('wakeup_suspend_is_active on:' + on);
            expect(on).assertTrue();
            power.suspend();
            let off = power.isActive();
            console.info('wakeup_suspend_is_active off:' + off);
            expect(off).assertFalse();
        } catch (e) {
            console.info('wakeup_suspend_is_active:' + e);
            expect().assertFail();
        }
    })

    /**
     * @tc.number PowerMgrPowerUnitTest018
     * @tc.name wakeup_invalid
     * @tc.desc wakeup Input invalid parameter
     */
    it('wakeup_invalid', 0, function () {
        try {
            power.wakeup(0)
        } catch (e) {
            console.info('wakeup_invalid code:' + e.code + "msg:" + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }

        try {
            // The number of parameters does not match
            power.wakeup('', 0)
        } catch (e) {
            console.info('wakeup_invalid code:' + e.code + "msg:" + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
    })
});
