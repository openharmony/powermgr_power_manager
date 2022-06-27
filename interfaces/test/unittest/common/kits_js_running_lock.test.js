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

import runningLock from '@ohos.runninglock';

import {describe, beforeAll, beforeEach, afterEach, afterAll, it, expect} from 'deccjsunit/index'

describe('PowerMgrRunningLockUnitTest', function () {
    beforeAll(function() {

        /*
         * @tc.setup: setup invoked before all test cases
         */
        console.info('PowerMgrRunningLockUnitTest beforeAll called');
    })

    afterAll(function() {

        /*
         * @tc.teardown: teardown invoked after all test cases
         */
        console.info('PowerMgrRunningLockUnitTest afterAll called');
    })

    beforeEach(function() {

        /*
         * @tc.setup: setup invoked before each test case
         */
        console.info('PowerMgrRunningLockUnitTest beforeEach called');
    })

    afterEach(function() {
        
        /*
         * @tc.teardown: teardown invoked after each test case
         */
        console.info('PowerMgrRunningLockUnitTest afterEach called');
    })

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
            done();
        })
        .catch(error => {
            console.log('create_running_lock_promise_test error: ' + error);
            expect().assertFail();
            done();
        })
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
                done();
            } else {
                console.log('create_running_lock_callback_test: ' + error);
                expect().assertFail();
                done();
            }
        })
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
            done();
        })
        .catch(error => {
            console.log('running_lock_lock_test error: ' + error);
            expect().assertFail();
            done();
        })
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
            done();
        })
        .catch(error => {
            console.log('running_lock_isused_test error: ' + error);
            expect().assertFail();
            done();
        })
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
            done();
        })
        .catch(error => {
            console.log('running_lock_unlock_test error: ' + error);
            expect().assertFail();
            done();
        })
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
            done();
        })
        .catch(error => {
            console.log('is_runninglock_type_supported_test_1 error: ' + error);
            expect().assertFail();
            done();
        })
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
            done();
        })
        .catch(error => {
            console.log('is_runninglock_type_supported_promise_test_2 error: ' + error);
            expect().assertFail();
            done();
        })
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
                done();
            } else {
                console.log('is_runninglock_type_supported_callback_test_3: ' + error);
                expect().assertFail();
                done();
            }
        })
    })
})
