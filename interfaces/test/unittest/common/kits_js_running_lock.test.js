/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

import runningLock from '@ohos.runningLock';
import { describe, it, expect } from 'deccjsunit/index'

describe('PowerMgrRunningLockUnitTest', function () {
    /**
     * @tc.number PowerMgrRunningLockUnitTest001
     * @tc.name create_running_lock_promise_test
     * @tc.desc createRunningLock
     */
    it('create_running_lock_promise_test', 0, async function (done) {
        runningLock.createRunningLock("running_lock_test_1", runningLock.RunningLockType.BACKGROUND)
            .then(runninglock => {
                expect(runninglock !== null).assertTrue();
                console.info('create_running_lock_promise_test success');
            })
            .catch(error => {
                console.log('create_running_lock_promise_test error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest002
     * @tc.name create_running_lock_callback_test
     * @tc.desc createRunningLock
     */
    it('create_running_lock_callback_test', 0, async function (done) {
        runningLock.createRunningLock("running_lock_test_2", runningLock.RunningLockType.BACKGROUND,
            (error, runninglock) => {
                if (typeof error === "undefined") {
                    console.info('create_running_lock_callback_test: runningLock is ' + runninglock);
                    expect(runninglock !== null).assertTrue();
                    var used = runninglock.isUsed();
                    console.info('create_running_lock_callback_test is used: ' + used);
                    expect(used).assertFalse();
                    runninglock.lock(500);
                    used = runninglock.isUsed();
                    console.info('after lock create_running_lock_callback_test is used: ' + used);
                    expect(used).assertTrue();
                    console.info('create_running_lock_callback_test success');
                } else {
                    console.log('create_running_lock_callback_test: ' + error);
                    expect().assertFail();
                    done();
                }
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest003
     * @tc.name running_lock_lock_test
     * @tc.desc lock
     */
    it('running_lock_lock_test', 0, async function (done) {
        runningLock.createRunningLock("running_lock_test_3", runningLock.RunningLockType.BACKGROUND)
            .then(runninglock => {
                expect(runninglock !== null).assertTrue();
                var used = runninglock.isUsed();
                console.info('running_lock_lock_test is used: ' + used);
                expect(used).assertFalse();
                runninglock.lock(500);
                used = runninglock.isUsed();
                console.info('after lock running_lock_lock_test is used: ' + used);
                expect(used).assertTrue();
                console.info('running_lock_lock_test success');
            })
            .catch(error => {
                console.log('running_lock_lock_test error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest004
     * @tc.name running_lock_isused_test
     * @tc.desc isUsed
     */
    it('running_lock_isused_test', 0, async function (done) {
        runningLock.createRunningLock("running_lock_test_4", runningLock.RunningLockType.BACKGROUND)
            .then(runninglock => {
                expect(runninglock !== null).assertTrue();
                var used = runninglock.isUsed();
                console.info('running_lock_isused_test used: ' + used);
                expect(used).assertFalse();
                console.info('running_lock_isused_test success');
            })
            .catch(error => {
                console.log('running_lock_isused_test error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest005
     * @tc.name running_lock_unlock_test
     * @tc.desc unlock
     */
    it('running_lock_unlock_test', 0, async function (done) {
        runningLock.createRunningLock("running_lock_test_5", runningLock.RunningLockType.BACKGROUND)
            .then(runninglock => {
                expect(runninglock !== null).assertTrue();
                var used = runninglock.isUsed();
                console.info('running_lock_unlock_test is used: ' + used);
                expect(used).assertFalse();
                runninglock.lock(500);
                used = runninglock.isUsed();
                console.info('after lock running_lock_unlock_test is used: ' + used);
                expect(used).assertTrue();
                runninglock.unlock();
                used = runninglock.isUsed();
                console.info('after unlock running_lock_unlock_test is used: ' + used);
                expect(used).assertFalse();
                console.info('running_lock_unlock_test success');
            })
            .catch(error => {
                console.log('running_lock_unlock_test error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest006
     * @tc.name enum_runningLock_type_background_test
     * @tc.desc Runninglock type is BACKGROUND
     */
    it('enum_runningLock_type_background_test', 0, function () {
        var runningLockType = runningLock.RunningLockType.BACKGROUND;
        console.info('runningLockType = ' + runningLockType);
        expect(runningLockType == 1).assertTrue();
        console.info('enum_runningLock_type_background_test success');
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest007
     * @tc.name enum_runningLock_type_proximityscreencontrol_test
     * @tc.desc Runninglock type is PROXIMITY_SCREEN_CONTROL
     */
    it('enum_runningLock_type_proximityscreencontrol_test', 0, function () {
        var runningLockType = runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL;
        console.info('runningLockType = ' + runningLockType);
        expect(runningLockType == 2).assertTrue();
        console.info('enum_runningLock_type_proximityscreencontrol_test success');
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest008
     * @tc.name is_runninglock_type_supported_promise_test_1
     * @tc.desc isRunningLockTypeSupported
     */
    it('is_runninglock_type_supported_promise_test_1', 0, async function (done) {
        runningLock.isRunningLockTypeSupported(runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL)
            .then(supported => {
                console.info('is_runninglock_type_supported_test_1 PROXIMITY_SCREEN_CONTROL supported is ' + supported);
                expect(supported).assertTrue();
                console.info('is_runninglock_type_supported_test_1 success');
            })
            .catch(error => {
                console.log('is_runninglock_type_supported_test_1 error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest009
     * @tc.name is_runninglock_type_supported_promise_test_2
     * @tc.desc isRunningLockTypeSupported
     */
    it('is_runninglock_type_supported_promise_test_2', 0, async function (done) {
        runningLock.isRunningLockTypeSupported(runningLock.RunningLockType.BACKGROUND)
            .then(supported => {
                console.info('is_runninglock_type_supported_promise_test_2 BACKGROUND supported is ' + supported);
                expect(supported).assertTrue();
                console.info('is_runninglock_type_supported_promise_test_2 success');
            })
            .catch(error => {
                console.log('is_runninglock_type_supported_promise_test_2 error: ' + error);
                expect().assertFail();
                done();
            })
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest010
     * @tc.name is_runninglock_type_supported_callback_test_3
     * @tc.desc isRunningLockTypeSupported
     */
    it('is_runninglock_type_supported_callback_test_3', 0, async function (done) {
        runningLock.isRunningLockTypeSupported(runningLock.RunningLockType.BACKGROUND, (error, supported) => {
            if (typeof error === "undefined") {
                console.info('is_runninglock_type_supported_callback_test_3 BACKGROUND supported is ' + supported);
                expect(supported).assertTrue();
                console.info('is_runninglock_type_supported_callback_test_3 success');
            } else {
                console.log('is_runninglock_type_supported_callback_test_3: ' + error);
                expect().assertFail();
                done();
            }
        })
        done();
    })

    // New interface
    /**
     * @tc.number PowerMgrRunningLockUnitTest011
     * @tc.name create_promise
     * @tc.desc Create lock, hold lock, unlock
     */
    it('create_promise', 0, async function (done) {
        try {
            let isExec = false;
            runningLock.create("create_promise", runningLock.RunningLockType.BACKGROUND)
                .then((error, runninglock) => {
                    isExec = true;
                    expect(typeof error === "undefined").assertTrue();
                    expect(runninglock !== null).assertTrue();
                    let holding = runninglock.isHolding();
                    console.info('create_promise holding false:' + holding);
                    expect(holding).assertFalse();
                    runninglock.hold(1000); // hold 1000ms
                    holding = runninglock.isHolding();
                    console.info('create_promise holding true:' + holding);
                    expect(holding).assertTrue();
                    runninglock.unhold();
                    expect(runninglock.isHolding()).assertFalse();
                }).finally(() => {
                    expect(isExec).assertTrue();
                })
        } catch (e) {
            console.info('create_promise error:' + e);
            expect().assertFail();
        }
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest012
     * @tc.name create_promise_invalid
     * @tc.desc Create lock input invalid value
     */
    it('create_promise_invalid', 0, async function (done) {
        try {
            runningLock.create(0, runningLock.RunningLockType.BACKGROUND)
                .then((error, runninglock) => {
                    expect().assertFail();
                })
        } catch (e) {
            console.info('create_promise_invalid code:' + e.code + "msg:" + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest013
     * @tc.name create_callback
     * @tc.desc Create lock, hold lock, unlock
     */
    it('create_callback', 0, async function (done) {
        try {
            runningLock.create("create_callback", runningLock.RunningLockType.BACKGROUND,
                (error, runninglock) => {
                    expect(typeof error === "undefined").assertTrue();
                    expect(runninglock !== null).assertTrue();
                    runninglock.hold(1000); // hold 1000ms
                    let holding = runninglock.isHolding();
                    console.info('create_callback holding true:' + holding);
                    expect(holding).assertTrue();
                    runninglock.unhold();
                    holding = runninglock.isHolding();
                    expect(holding).assertFalse();
                    console.info('create_callback holding false:' + holding);
                });
        } catch (e) {
            console.info('create_callback error:' + e);
            expect().assertFail();
        }
        done();
    })

    /**
     * @tc.number PowerMgrRunningLockUnitTest014
     * @tc.name create_callback_invalid
     * @tc.desc Create lock input invalid value
     */
    it('create_callback_invalid', 0, async function (done) {
        try {
            runningLock.create("create_callback_invalid", "invalid",
                (error, runninglock) => {
                    expect().assertFail();
                });
        } catch (e) {
            console.info('create_promise_invalid code:' + e.code + "msg:" + e.message);
            // 401: Invalid input parameter
            expect(e.code === 401).assertTrue();
        }
        done();
    })
})
