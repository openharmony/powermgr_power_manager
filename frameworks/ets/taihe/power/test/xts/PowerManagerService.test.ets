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

import { describe, it, expect, TestType, Size, Level } from "../hypium/index";
import { BusinessError } from '@ohos.base'
import hilog from '@ohos.hilog'
import Utils from './Util.test';
import power_ from '@ohos.power'

let domain: number = 0x0000;
let tag: string = 'testTag';
const ERROR_CODE = 202;

export default function PowerManagerServiceTest() {

  describe("PowerManagerServiceTest", (): void => {
    hilog.info(domain, tag, '%{public}s', 'describe start');
    it("PowerManagerServiceTest0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0100 start');
        try {
          let isActive = power_.isActive();
          hilog.info(domain, tag, '%{public}s', JSON.stringify(isActive));
          hilog.info(domain, tag, '%{public}s', typeof isActive);
          expect(typeof isActive).assertEqual('boolean');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("PowerManagerServiceTest0200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0200 start');
        let TAG = 'PowerManagerServiceTest0200';
        try {
          power_.wakeup("PowerManagerServiceTest0200");
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0300 start');
        let TAG = 'PowerManagerServiceTest0300';
        try {
          power_.suspend(true);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0400 start');
        let TAG = 'PowerManagerServiceTest0400';
        try {
          power_.suspend(false);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0500", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0500 start');
        let TAG = 'PowerManagerServiceTest0500';
        try {
          power_.suspend();
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0600", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0600 start');
        let TAG = 'PowerManagerServiceTest0600';
        try {
          let currentMode = power_.getPowerMode();
          console.info(TAG + ' currentMode:' + currentMode);
          expect(currentMode >= power_.DevicePowerMode.MODE_NORMAL &&
            currentMode <= power_.DevicePowerMode.MODE_EXTREME_POWER_SAVE).assertTrue();
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("PowerManagerServiceTest0700", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0700 start');
        let TAG = 'PowerManagerServiceTest0700';
        try {
          await power_.setPowerMode(power_.DevicePowerMode.MODE_PERFORMANCE);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0800", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0800 start');
        let TAG = 'PowerManagerServiceTest0800';
        try {
          await power_.setPowerMode(power_.DevicePowerMode.MODE_PERFORMANCE);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest0900", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0900 start');
        let TAG = 'PowerManagerServiceTest0900';
        try {
          let isStandby = power_.isStandby();
          console.info(TAG + ' end isStandby:' + isStandby);
          expect(isStandby).assertFalse();
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("PowerManagerServiceTest1000", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1000 start');
        let TAG = 'PowerManagerServiceTest1000';
        try {
          power_.setScreenOffTime(3000);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest1100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1100 start');
        let TAG = 'PowerManagerServiceTest1100';
        let devicePowerMode = power_.DevicePowerMode.MODE_NORMAL;
        expect(devicePowerMode === 600).assertTrue();
        console.info(TAG + ' end');
        done();
      })
    it("PowerManagerServiceTest1200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1200 start');
        let TAG = 'PowerManagerServiceTest1200';
        let devicePowerMode = power_.DevicePowerMode.MODE_POWER_SAVE;
        expect(devicePowerMode === 601).assertTrue();
        console.info(TAG + ' end');
        done();
      })
    it("PowerManagerServiceTest1300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1300 start');
        let TAG = 'PowerManagerServiceTest1300';
        let devicePowerMode = power_.DevicePowerMode.MODE_PERFORMANCE;
        expect(devicePowerMode === 602).assertTrue();
        console.info(TAG + ' end');
        done();
      })
    it("PowerManagerServiceTest1400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1400 start');
        let TAG = 'PowerManagerServiceTest1400';
        let devicePowerMode = power_.DevicePowerMode.MODE_EXTREME_POWER_SAVE;
        expect(devicePowerMode === 603).assertTrue();
        console.info(TAG + ' end');
        done();
      })
    it("PowerManagerServiceTest1500", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1500 start');
        let TAG = 'PowerManagerServiceTest1500';
        try {
          power_.shutdown("PowerManagerServiceTest1500");
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
        
      })
    it("PowerManagerServiceTest1600", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1600 start');
        let TAG = 'PowerManagerServiceTest1600';
        try {
          power_.reboot("PowerManagerServiceTest1600");
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
        
      })
    it("PowerManagerServiceTest1700", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1700 start');
        let TAG = 'PowerManagerServiceTest1700';
        try {
          power_.hibernate(true);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("PowerManagerServiceTest1800", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest1800 start');
        let TAG = 'PowerManagerServiceTest1800';
        try {
          power_.hibernate(false);
          console.info(TAG + ' end');
          done();
        } catch (e : BusinessError) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
  })
  hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest end');
}