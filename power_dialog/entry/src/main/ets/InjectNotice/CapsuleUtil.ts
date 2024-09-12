/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

import process from '@ohos.process';
import PluginMgr from '@ohos.pluginComponent';
import pluginComponentManager from '@ohos.pluginComponent';
import { AsyncCallback } from '@ohos.base';

const TAG = 'InjectNotice';
const PLUGIN_NAME = 'pages/capsuleIcon.js';

export class CapsuleUtil {
  public static readonly APP_NAME: string = 'com.ohos.powerdialog';
  private static instance: CapsuleUtil;
  private constructor() {}

  public static getInstance(): CapsuleUtil {
    if (!CapsuleUtil.instance) {
      console.debug(TAG, `instance begin init`);
      CapsuleUtil.instance = new CapsuleUtil();
    }
    console.debug(TAG, `instance return obj:${CapsuleUtil.instance}`);
    return CapsuleUtil.instance;
  }

  /**
   * peocess capsule visible or not
   *
   * @param isVisible visible status
   */
  private pushToPluginMgr(extraData: pluginComponentManager.KVObject, logInfo: AsyncCallback<void>): void {
    console.debug(TAG, `pushToPluginMgr push begin `);
    let pushData:PluginMgr.PushParameterForStage = {
      owner: {
        bundleName: CapsuleUtil.APP_NAME,
      },
      target: {
        bundleName: CapsuleUtil.APP_NAME,
      },
      name: PLUGIN_NAME,
      data: {},
      extraData: extraData
    };
    console.debug(TAG, `pushToPluginMgr push content: ${JSON.stringify(pushData)} `);
    PluginMgr.push(pushData, logInfo);
  }
  
  public processCapsule(isVisible: boolean): void {
    this.pushToPluginMgr({
      requestVisible: isVisible,
      requestPid: process.pid
    }, (err, data) => {
      console.info(TAG, 'push complete isVisible: ' + isVisible);
      console.log('push_callback:err:', JSON.stringify(err));
      console.log('push_callback:data:', JSON.stringify(data));
    });
  }

  /**
   * close panel
   */
  public closePanel(): void {
    this.pushToPluginMgr({
      requestVisible: false,
      requestCloseWindow: true,
      requestPid: process.pid
    }, (err, data) => {
        console.info(TAG, 'push close panel complete');
        console.log('push_callback:err:', JSON.stringify(err));
        console.log('push_callback:data:', JSON.stringify(data));
    });
  }

  /**
   * close panel
   */
  public closeWindow(): void {
    this.pushToPluginMgr({
      requestCloseWindow: true
    }, (err, data) => {
        console.info(TAG, 'push close window complete');
        console.log('push_callback:err:', JSON.stringify(err));
        console.log('push_callback:data:', JSON.stringify(data));
    });
  }
}