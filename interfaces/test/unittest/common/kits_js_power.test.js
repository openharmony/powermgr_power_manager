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

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

function sleep(time){
    return new Promise((resolve) => setTimeout(resolve, time));
}

describe('PowerMgrPowerUnitTest', function () {
    beforeAll(function() {

        /*
         * @tc.setup: setup invoked before all test cases
         */
        console.info('PowerMgrPowerUnitTest beforeAll called');
    })

    afterAll(function() {

        /*
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('PowerMgrPowerUnitTest afterAll called');
    })

    beforeEach(function() {

        /*
         * @tc.setup: setup invoked before each test case
         */
        console.info('PowerMgrPowerUnitTest beforeEach called');
    })

    afterEach(function() {
        
        /*
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('PowerMgrPowerUnitTest afterEach called');
    })

    /**
     * @tc.number PowerMgrPowerUnitTest001
     * @tc.name Screen off
     * @tc.desc Screen off
     */
    it('power_suspend', 0, async function () {
        power.isScreenOn().then(screenOn => {
            console.info('power_suspend01: The current screenOn is ' + screenOn);
            if (!screenOn) { 
                power.wakeupDevice("power_suspend");
            } 
        }).catch(error => {
            console.log('power_suspend error: ' + error);
            expect().assertFail();
        })
        await sleep(1000);
        power.suspendDevice();
        await sleep(1000);
        power.isScreenOn().then(screenOn => {
            console.info('power_suspend02: The current screenOn is ' + screenOn);
            expect(screenOn).assertFalse();
            console.info('power_suspend success');
        }).catch(error => {
            console.log('power_suspend error: ' + error);
            expect().assertFail();
        })
    })

    /**
     * @tc.number PowerMgrPowerUnitTest002
     * @tc.name Screen on
     * @tc.desc Screen on
     */
    it('power_wake_up_success', 0, async function () {
        power.isScreenOn().then(screenOn => {
            console.info('The current screenOn is ' + screenOn);
            if (screenOn) {
                power.suspendDevice();
            }
        }).catch(error => {
            console.log('power_wake_up_success error: ' + error);
            expect().assertFail();
        })
        await sleep(1000);
        power.wakeupDevice("power_wake_up_success");
        await sleep(1000);
        power.isScreenOn().then(screenOn => {
            console.info('The current screenOn is ' + screenOn);
            expect(screenOn).assertTrue();
            console.info('power_wake_up_success success');
        }).catch(error => {
            console.log('power_wake_up_success error: ' + error);
            expect().assertFail();
        })
    })

    /**
     * @tc.number PowerMgrPowerUnitTest003
     * @tc.name Screen on
     * @tc.desc Screen on
     */
    it('power_wake_up_fail', 0, async function () {
        power.isScreenOn().then(screenOn => {
            console.info('power_wake_up_fail01: The current screenOn is ' + screenOn);
            if (screenOn) {
                power.suspendDevice();
            }  
        })
        .catch(error => {
            console.log('power_wake_up_fail error: ' + error);
            expect().assertFail();
        })
        await sleep(1000);
        try {
            power.wakeupDevice(1)
        } catch (error) {
            console.log('power_wake_up_fail wakeupDevice error: ' + error);
        }
        await sleep(1000);
        power.isScreenOn().then(screenOn => {
            console.info('power_wake_up_fail02: The current screenOn is ' + screenOn);
            expect(screenOn).assertFalse();
            console.info('power_wake_up_fail success');
        }).catch(error => {
            console.log('power_wake_up_fail error: ' + error);
            expect(error === "Wrong argument type. string expected.").assertTrue();
        })
    })

    /**
     * @tc.number PowerMgrPowerUnitTest004
     * @tc.name power_get_set_power_mode_promise
     * @tc.desc Get The mode the device
     */
     it('power_get_set_power_mode_promise', 0, async function (done) {
        power.getPowerMode()
        .then(powerMode => {
            console.info('power_get_set_power_mode_promise original power mode is ' + powerMode);
        })
        .catch(error => {
            console.log('power_get_set_power_mode_promise error: ' + error);
            expect().assertFail();
            done();
        })
        power.setPowerMode(power.DevicePowerMode.MODE_POWER_SAVE)
            .then(() => {
                power.getPowerMode()
                .then(powerMode => {
                    console.info('power_get_set_power_mode_promise changed power mode is ' + powerMode);
                    expect(powerMode === power.DevicePowerMode.MODE_POWER_SAVE).assertTrue();
                    console.info('power_get_set_power_mode_promise success');
                })
                .catch(error => {
                    console.log('power_get_set_power_mode_promise error: ' + error);
                    expect().assertFail();
                    done();
                })
            })
        power.setPowerMode(power.DevicePowerMode.MODE_NORMAL);
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest005
     * @tc.name power_get_set_power_mode_callback
     * @tc.desc Get The mode the device
     */
    it('power_get_set_power_mode_callback', 0, async function (done) {
        power.getPowerMode((error, powerMode) => {
            if (typeof error === "undefined") {
                console.info('power_get_set_power_mode_callback original power mode is ' + powerMode);
            } else {
                console.log('power_get_set_power_mode_callback: ' + error);
                expect().assertFail();
                done();
            }    
        })
        power.setPowerMode(power.DevicePowerMode.MODE_POWER_SAVE, () => {
            power.getPowerMode((error, powerMode) => {
                if (typeof error === "undefined") {
                    console.info('power_get_set_power_mode_callback changed power mode is ' + powerMode);
                    expect(powerMode === power.DevicePowerMode.MODE_POWER_SAVE).assertTrue();
                    console.info('power_get_set_power_mode_callback success');
                } else {
                    console.log('power_get_set_power_mode_callback: ' + error);
                    expect().assertFail();
                    done();
                }    
            })
        })
        power.setPowerMode(power.DevicePowerMode.MODE_NORMAL);
        done();
    })

    /**
     * @tc.number PowerMgrPowerUnitTest006
     * @tc.name power_device_power_mode_MODE_NORMAL
     * @tc.desc Get device power mode MODE_NORMAL
     */
     it('power_device_power_mode_normal', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_NORMAL;
        console.info('MODE_NORMAL = ' + devicePowerMode);
        expect(devicePowerMode === 600).assertTrue();
      })

    /**
     * @tc.number PowerMgrPowerUnitTest007
     * @tc.name power_device_power_mode_MODE_POWER_SAVE
     * @tc.desc Get device power mode MODE_POWER_SAVE
     */
     it('power_device_power_mode_power_save', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_POWER_SAVE;
        console.info('MODE_POWER_SAVE = ' + devicePowerMode);
        expect(devicePowerMode === 601).assertTrue();
      })

    /**
     * @tc.number PowerMgrPowerUnitTest008
     * @tc.name power_device_power_mode_MODE_PERFORMANCE
     * @tc.desc Get device power mode MODE_PERFORMANCE
     */
     it('power_device_power_mode_performance', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_PERFORMANCE;
        console.info('MODE_PERFORMANCE = ' + devicePowerMode);
        expect(devicePowerMode === 602).assertTrue();
      })

    /**
     * @tc.number PowerMgrPowerUnitTest009
     * @tc.name power_device_power_mode_MODE_EXTREME_POWER_SAVE
     * @tc.desc Get device power mode MODE_EXTREME_POWER_SAVE
     */
     it('power_device_power_mode_extreme_power_save', 0, function () {
        let devicePowerMode = power.DevicePowerMode.MODE_EXTREME_POWER_SAVE;
        console.info('MODE_EXTREME_POWER_SAVE = ' + devicePowerMode);
        expect(devicePowerMode === 603).assertTrue();
      })

    /**
     * @tc.number PowerMgrPowerUnitTest010
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
            done();
        })
        .catch(error => {
            console.log('power_is_screen_on_promise_test error: ' + error);
            expect().assertFail();
            done();
        })
    })

    /**
     * @tc.number PowerMgrPowerUnitTest011
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
                done();
            } else {
                console.log('power_is_screen_on_callback_test: ' + error);
                expect().assertFail();
                done();
            }
        })
    })
});