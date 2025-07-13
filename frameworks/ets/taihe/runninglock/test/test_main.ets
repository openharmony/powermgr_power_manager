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

import runningLock from "@ohos.runningLock";

function main() {
    try {
        console.info('create async RunningLock start');
        runningLock.create("RunningLockTest", runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL, (error: Error, lock_?: runningLock.RunningLock) => {
            if (!error.message) {
              console.info('create async RunningLock success');
              lock_?.hold(30000);
              let isHold = lock_?.isHolding();
              console.info('isHold RunningLock after hold ${isHold}');
              lock_?.unhold();
              isHold = lock_?.isHolding();
              console.info('isHold RunningLock after unhold ${isHold}');
              console.info('create async RunningLock end');
            } else {
              console.error('create async RunningLock ERROR ', error);
            }
        })

        console.info('create async promise RunningLock start');
        let lock_ = await runningLock.create("RunningLockTest", runningLock.RunningLockType.PROXIMITY_SCREEN_CONTROL);
        lock_.hold(30000);
        let isHold = lock_.isHolding();
        console.info('isHold RunningLock after hold ${isHold}');
        lock_.unhold();
        isHold = lock_.isHolding();
        console.info('isHold RunningLock after unhold ${isHold}');
        console.info('create async promise RunningLock end');
    } catch (err) {
        console.error("test runningLock err: " + err)
    }
}