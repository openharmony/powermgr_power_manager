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

import { testPower } from "./test_power";

function main() {
    try {
        testPower.testsuspend();
        console.info('testsuspend end');
        testPower.testwakeup();
        console.info('testwakeup end');
        testPower.testgetPowerMode();
        console.info('testgetPowerMode end');
        testPower.testisActive();
        console.info('testisActive end');
        testPower.testsetPowerModeCallback();
        console.info('testsetPowerModeCallback end');
        testPower.testsetPowerModePromise();
        console.info('testsetPowerModePromise end');
        testPower.testsetScreenOffTime();
        console.info('testsetScreenOffTime end');
        testPower.testsuspendWiteParam();
        console.info('testsuspendWiteParam end');
    } catch (err) {
        console.error("testPower err: " + err)
    }
  
}