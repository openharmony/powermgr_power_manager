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
import GlobalContext from '../common/GlobalContext';
import * as notice_sub from '../InjectNotice/InjectNoticeStub';
import {NOTICE_ID, injectNoticeUtil } from '../InjectNotice/InjectNoticeUtil';

const TAG = 'InjectNotice';
const ACCESS_TYPE_MASK = 0b11;
const SHIFT_DIGIT = 27;
const TOKEN_NATIVE = 1;

export default class InjectNoticeAbility extends extension {
  /**
   * Lifecycle function, called back when a service extension is started for initialization.
   */
  onCreate(want): void {
    try {
      console.debug(TAG, 'InjectNoticeAbility onCreate' + JSON.stringify(want));
      GlobalContext.getContext().setObject('appcontext', this.context.getApplicationContext());
      injectNoticeUtil.init();
    } catch (e) {
      console.error(TAG, 'InjectNoticeAbility onCreate', e);
    }
    console.debug(TAG, 'InjectNoticeAbility onCreate end');
  }

  onConnect(want): rpc.RemoteObject {
    let ret: rpc.RemoteObject = null;
    try {
      console.debug(TAG, 'onConnect want: ' + JSON.stringify(want));
      ret = new notice_sub.InjectNoticeStub('InjectNoticeStub');
    } catch (e) {
      console.error(TAG, 'InjectNoticeAbility onConnect', e);
    }
    return ret;
  }

  onDisconnect(want): void {
    console.info(TAG, 'onDisconnect');
  }

  /**
   * Lifecycle function, called back when a service extension is started or recall.
   */
  onRequest(want, startId): void {
    try {
      console.debug(TAG, `onRequest startId: ${startId} want:${JSON.stringify(want)}`);
      if ('noticeId' in want.parameters) {
        console.debug(TAG, `onRequest noticeId has value`);
        if (want.parameters.noticeId === NOTICE_ID) {
          console.debug(TAG, `onRequest startId: ${startId} close notice`);
          injectNoticeUtil.cancelAuthorization();
          injectNoticeUtil.cancelNotificationById(NOTICE_ID);
          return;
        }
      }
    } catch (e) {
      console.error(TAG, 'InjectNoticeAbility onRequest', e);
    }
  }
  /**
   * Lifecycle function, called back before a service extension is destroyed.
   */
  onDestroy(): void {
    console.debug(TAG, 'onDestroy.');
    injectNoticeUtil.unInit();
  }

  private isSystemAbility(callingTokenId: number): boolean {
    let type: number = ACCESS_TYPE_MASK & (callingTokenId >> SHIFT_DIGIT);
    console.debug(TAG, 'InjectNoticeAbility isSystemAbility, type:' + type);
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
    console.debug(TAG, `verify ${permissionName}, success`);
    return true;
  }
}