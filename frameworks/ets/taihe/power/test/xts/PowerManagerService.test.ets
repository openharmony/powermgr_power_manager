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
import power from '@ohos.power'

let domain: number = 0x0000; //日志标识,0x0000作为测试框架的业务标识
let tag: string = 'testTag'; //日志标识字符串,作为tag标识当前runner类下的测试行为
const ERROR_CODE = 202;

export default function PowerManagerServiceTest() {

  describe("PowerManagerServiceTest", (): void => {
    hilog.info(domain, tag, '%{public}s', 'describe start');
    it("PowerManagerServiceTest0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest0100 start');
        try {
          let isActive = power.isActive();
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
          power.wakeup("PowerManagerServiceTest0200");
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
          power.suspend(true);
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
          power.suspend(false);
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
          power.suspend();
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
          let currentMode = power.getPowerMode();
          console.info(TAG + ' currentMode:' + currentMode);
          expect(currentMode >= power.DevicePowerMode.MODE_NORMAL &&
            currentMode <= power.DevicePowerMode.MODE_EXTREME_POWER_SAVE).assertTrue();
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
          power.setPowerMode(power.DevicePowerMode.MODE_PERFORMANCE)
            .then(() => {
              console.info('set power mode to MODE_PERFORMANCE');
              expect().assertFail();
            })
            .catch((error) => {
              console.info(TAG + ` setPowerMode err: ${error.code} ${error.message}`);
              expect(error.code).assertEqual(ERROR_CODE);
            });
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
          let isStandby = power.isStandby();
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
          power.setScreenOffTime(3000);
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
        let devicePowerMode = power.DevicePowerMode.MODE_NORMAL;
        expect(devicePowerMode === 600).assertTrue();
        devicePowerMode = power.DevicePowerMode.MODE_POWER_SAVE;
        expect(devicePowerMode === 601).assertTrue();
        devicePowerMode = power.DevicePowerMode.MODE_PERFORMANCE;
        expect(devicePowerMode === 602).assertTrue();
        devicePowerMode = power.DevicePowerMode.MODE_EXTREME_POWER_SAVE;
        expect(devicePowerMode === 603).assertTrue();
        console.info(TAG + ' end');
        done();
      })
  })
  hilog.info(domain, tag, '%{public}s', 'PowerManagerServiceTest end');
}