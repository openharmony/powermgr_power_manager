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

import extension from '@ohos.app.ability.ServiceExtensionAbility';
import type { Permissions } from '@ohos.abilityAccessCtrl';
import abilityAccessCtrl from '@ohos.abilityAccessCtrl';
import rpc from '@ohos.rpc';
import * as notice_sub from '../InjectNotice/InjectNoticeStub';
import { injectNoticeUtil } from '../InjectNotice/InjectNoticeUtil';


const TAG = "InjectNotice";
const ACCESS_TYPE_MASK = 0b11;
const SHIFT_DIGIT = 27;
const TOKEN_NATIVE = 1;

export default class InjectNoticeAbility extends extension {
  /**
   * Lifecycle function, called back when a service extension is started for initialization.
   */
  onCreate(want) {
    console.info(TAG, 'InjectNoticeAbility onCreate' + want.abilityName);
    try {
      injectNoticeUtil.init();
    } catch (e) {
      console.error(TAG, 'InjectNoticeAbility onCreate' + want.abilityName);
    }
    console.info(TAG, "InjectNoticeAbility onCreate end");

  }

  onConnect(want): rpc.RemoteObject {
    console.info(TAG, 'onConnect want: ' + JSON.stringify(want));
    let callingTokenId: number = rpc.IPCSkeleton.getCallingTokenId();
    return new notice_sub.InjectNoticeStub('InjectNoticeStub');
  }

  onDisconnect(want): void {
    console.info(TAG, 'onDisconnect');
  }

  /**
   * Lifecycle function, called back when a service extension is started or recall.
   */
  onRequest(want, startId): void {
    console.info(TAG, 'onRequest');
  }
  /**
   * Lifecycle function, called back before a service extension is destroyed.
   */
  onDestroy(): void {
    console.info(TAG, 'onDestroy.');
  }

  private isSystemAbility(callingTokenId: number): boolean {
    let type: number = ACCESS_TYPE_MASK & (callingTokenId >> SHIFT_DIGIT);
    console.info(TAG, 'InjectNoticeAbility isSystemAbility, type:' + type);
    return type === TOKEN_NATIVE;
  }

  private checkPermission(tokenID: number, permissionName: Permissions): boolean {
    let aac = abilityAccessCtrl.createAtManager();
    try {
      let grantStatus = aac.verifyAccessTokenSync(tokenID, permissionName);
      if (grantStatus === abilityAccessCtrl.GrantStatus.PERMISSION_DENIED) {
        console.error(TAG, `verify ${permissionName} fail`);
      }
    } catch (error) {
      console.error(TAG, `verify ${permissionName}, ${error}`);
    }
    console.info(TAG, `verify ${permissionName}, success`);
    return true;
  }
}
