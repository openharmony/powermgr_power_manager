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

import { describe, it, expect, TestType, Size, Level, beforeAll} from "../../index";
import { BusinessError } from '@ohos.base'
import hilog from '@ohos.hilog'
import Utils from './Util.test';
import runningLock from '@ohos.runningLock'

let domain: number = 0x0000;
let tag: string = 'testTag';
const ERROR_CODE = 201;

export default function RunningLockTest() {

  describe("RunningLockTest", (): void => {
    hilog.info(domain, tag, '%{public}s', 'describe start');
    it("RunningLockTest0100", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'RunningLockTest0100 start');
        let TAG = 'RunningLockTest0100';
        try {
          runningLock.create("RunningLockTest0100", runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL, (error: Error, lock_?: runningLock.RunningLock) => {
            if (!error.message) {
              console.info(TAG + 'create async RunningLock success')
              lock_?.hold(30000);
              let isHold = lock_?.isHolding();
              lock_?.unhold();
              isHold = lock_?.isHolding();
              hilog.info(domain, tag, '%{public}s', JSON.stringify(isHold));
              hilog.info(domain, tag, '%{public}s', typeof isHold);
              expect(typeof isHold).assertEqual('boolean');
            } else {
              console.info(TAG + 'create async RunningLock ERROR ', error);
            }
          })
          console.info(TAG + ' end');
          done();
        } catch (e) {
          if (!(e instanceof BusinessError)){
            throw e;
          }
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("RunningLockTest0200", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'RunningLockTest0200 start');
        let TAG = 'RunningLockTest0200';
        try {
          let lock_ = await runningLock.create("RunningLockTest0200", runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL);
          console.info(TAG + ' create async promise RunningLock success');
          lock_.hold(30000);
          let isHold = lock_.isHolding();
          lock_.unhold();
          isHold = lock_.isHolding();
          hilog.info(domain, tag, '%{public}s', JSON.stringify(isHold));
          hilog.info(domain, tag, '%{public}s', typeof isHold);
          expect(typeof isHold).assertEqual('boolean');
          console.info(TAG + ' end');
          done();
        } catch (e) {
          if (!(e instanceof BusinessError)) {
            throw e;
          }
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          expect(e.code).assertEqual(ERROR_CODE);
          done();
        }
      })
    it("RunningLockTest0300", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'RunningLockTest0300 start');
        let TAG = 'RunningLockTest0300';
        try {
          let runningLockType = runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL;
          expect(runningLockType === 2).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
    it("RunningLockTest0400", TestType.FUNCTION | Size.MEDIUMTEST | Level.LEVEL1,
      async (done: () => void): Promise<void> => {
        hilog.info(domain, tag, '%{public}s', 'RunningLockTest0400 start');
        let TAG = 'RunningLockTest0400';
        try {
          let runningLockType = runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL;
          let isSupported = runningLock.isSupported(runningLockType);
          expect(isSupported).assertTrue();
          console.info(TAG + ' end');
          done();
        } catch (e) {
          hilog.error(domain, tag, '%{public}s', JSON.stringify(e));
          done();
        }
      })
  })
  hilog.info(domain, tag, '%{public}s', 'RunningLockTest end');
}