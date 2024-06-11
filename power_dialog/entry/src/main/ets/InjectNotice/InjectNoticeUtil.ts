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

import notificationManager from '@ohos.notificationManager';
import notificationSubscribe from '@ohos.notificationSubscribe';
import inputEventClient from '@ohos.multimodalInput.inputEventClient';
import WantAgent, { WantAgent as _WantAgent } from '@ohos.wantAgent';
import CommonEventManager from '@ohos.commonEvent';
import Base from '@ohos.base';
import type context from '@ohos.app.ability.common';
import GlobalContext from '../common/GlobalContext';


const TAG: string = 'InjectNotice';
const LABEL: string = 'inject notice';
const EVENT_NAME: string = 'event_inject_close_notice';
export const NOTICE_ID: number = 100;

export enum InjectNoticeStaus {
  DEFAULT = 0,
  OPENING = 1,
  SERVER_CLOSE = 2,
  MAN_CLOSE = 3,
};

class InjectNoticeUtil {
  status: InjectNoticeStaus = InjectNoticeStaus.DEFAULT;
  isInit: boolean = false;
  subscriber: notificationSubscribe.NotificationSubscriber = {
    onCancel: this.onCancelCallBack,
  };
  wantAgentObj: _WantAgent = null;
  wantAgentInfo: WantAgent.WantAgentInfo = {
    wants: [
      {
        bundleName: 'com.ohos.powerdialog',
        abilityName: 'InjectNoticeAbility',
        action: '',
        parameters: {
          noticeId: NOTICE_ID,
          noticeLabel: LABEL,
        },
      }
    ],
    actionType: WantAgent.OperationType.START_ABILITY,
    requestCode: 0,
    wantAgentFlags: [WantAgent.WantAgentFlags.CONSTANT_FLAG],
  };
  removalWantAgentObj: _WantAgent = null;
  removalWantAgentInfo: WantAgent.WantAgentInfo = {
    wants: [
      {
        action: EVENT_NAME,
        parameters: {
          noticeId: NOTICE_ID,
          noticeLabel: LABEL,
        },
      }
    ],
    operationType: WantAgent.OperationType.SEND_COMMON_EVENT,
    requestCode: 0,
    wantAgentFlags: [WantAgent.WantAgentFlags.CONSTANT_FLAG],
  };
  commonEventSubscriber: CommonEventManager.CommonEventSubscriber = null;

  constructor() {

  }

  async init(): Promise<void> {
    console.info(TAG, `init begin`);
    if (this.isInit) {
      return;
    }
    // console.info(TAG, 'subscribe notification begin');
    /* await notificationSubscribe.subscribe(this.subscriber, (err: Base.BusinessError) => {
       if (err) {
         console.error(TAG + `Failed to subscribe notification. Code is ${err.code}, message is ${err.message}`);
         return;
       }
     });*/
    this.initRemovalWantAgent();
    this.initButtonWantAgent();
    this.createCommonEventSubscriber();
    console.info(TAG, 'init end');
    this.isInit = true;
  }

  private onCancelCallBack(data: notificationSubscribe.SubscribeCallbackData): void {
    console.info(TAG, 'Cancel callback:' + JSON.stringify(data));
  }

  async cancelNotificationById(id: number): Promise<void> {
    try {
      await notificationManager.cancel(id, LABEL);
      console.info(TAG, 'cancel notification success');
    } catch (err) {
      if (err) {
        console.error(TAG, 'cancel notification err: ' + JSON.stringify(err));
      }
    }
  }

  sendNotice(): void {
    console.debug(TAG, 'sendNotice begin==>');
    this.enableNotification();
    console.debug(TAG, 'sendNotice enableNotification end');
    console.debug(TAG, `wantAgentObj is ${this.wantAgentObj}`);
    console.debug(TAG, `removalWantAgentObj is ${this.removalWantAgentObj}`);
    let applicationContext = (GlobalContext.getContext().getObject('appcontext')) as context.ApplicationContext;
    let resourceManager = applicationContext.resourceManager;
    let imagePixelMap = resourceManager.getDrawableDescriptor($r('app.media.icon_notice')).getPixelMap();
    let notificationRequest: notificationManager.NotificationRequest = {
      id: NOTICE_ID,
      label: LABEL,
      isOngoing: true,
      isFloatingIcon: true,
      smallIcon: imagePixelMap,
      largeIcon: imagePixelMap,
      removalWantAgent: this.removalWantAgentObj,
      slotType: notificationManager.SlotType.SERVICE_INFORMATION,
      actionButtons: [
        {
          title: resourceManager.getStringSync($r('app.string.bn_notice_cancel')),
          wantAgent: this.wantAgentObj,
        }
      ],
      content: {
        notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT, // 普通文本类型通知
        normal: {
          title: resourceManager.getStringSync($r('app.string.notice_title')),
          text: resourceManager.getStringSync($r('app.string.notice_text')),
        },

      },
    };
    console.info(TAG, 'publish begin');
    notificationManager.publish(notificationRequest, (err: Base.BusinessError) => {
      if (err) {
        console.error(TAG, TAG + `Failed to publish notification. Code is ${err.code}, message is ${err.message}`);
        return;
      }
      console.info(TAG, 'Succeeded in publishing notification.');
    });
  }

  async initRemovalWantAgent(): Promise<void> {
    console.info(TAG, 'initRemovalWantAgent begin');
    if (this.removalWantAgentObj !== null) {
      console.info(TAG, 'RemovalWantAgent has init');
      return;
    }
    try {
      await WantAgent.getWantAgent(this.removalWantAgentInfo).then((data) => {
        this.removalWantAgentObj = data;
        console.info(TAG, `initRemovalWantAgent getWantAgent ok`);
      }).catch((err: Base.BusinessError) => {
        console.error(TAG, `initRemovalWantAgent getWantAgent failed, code: ${JSON.stringify(err.code)}, message: ${JSON.stringify(err.message)}`);
      });
    } catch (err: Base.BusinessError) {
      console.error(TAG, `initRemovalWantAgent getWantAgent catch failed, code: ${JSON.stringify(err.code)}, message: ${JSON.stringify(err.message)}`);
    }
    console.info(TAG, 'initRemovalWantAgent getWantAgent end');
  }

  async initButtonWantAgent(): Promise<void> {
    console.info(TAG, 'getWantAgent begin');
    if (this.wantAgentObj !== null) {
      console.info(TAG, 'has init getWantAgent');
      return;
    }
    try {
      await WantAgent.getWantAgent(this.wantAgentInfo).then((data) => {
        this.wantAgentObj = data;
        console.info(TAG, `getWantAgent ok`);
      }).catch((err: Base.BusinessError) => {
        console.error(TAG, `getWantAgent failed, code: ${JSON.stringify(err.code)}, message: ${JSON.stringify(err.message)}`);
      });
    } catch (err: Base.BusinessError) {
      console.error(TAG, `getWantAgent catch failed, code: ${JSON.stringify(err.code)}, message: ${JSON.stringify(err.message)}`);
    }
    console.info(TAG, 'getWantAgent end');
  }

  async enableNotification(): Promise<void> {
    console.debug(TAG, 'requestEnableNotification begin===>');
    await notificationManager.requestEnableNotification().then(() => {
      console.debug(TAG + 'requestEnableNotification success');
    }).catch((err: Base.BusinessError) => {
      console.error(TAG + `requestEnableNotification fail: ${JSON.stringify(err)}`);
    });
    console.debug(TAG, 'requestEnableNotification end===>');
  }

  async cancelAuthorization(): Promise<void> {
    console.debug(TAG, 'cancelAuthorization begin===>');
    try {
      inputEventClient.permitInjection(false);
    } catch(err: Base.BusinessError){
      console.error(TAG + `cancelAuthorization fail: ${JSON.stringify(err)}`);
    }
    console.debug(TAG, 'cancelAuthorization end===>');
  }

  createCommonEventSubscriber(): void {
    console.debug(TAG, 'createCommonEventSubscriber begin===>');
    if(this.commonEventSubscriber !== null) {
      console.debug(TAG, `commonEventSubscriber has init`);
      return;
    }
    let subscribeInfo: CommonEventManager.CommonEventSubscribeInfo = {
      events: [EVENT_NAME],
    };
    CommonEventManager.createSubscriber(subscribeInfo).then((commonEventSubscriber: CommonEventManager.CommonEventSubscriber) => {
      console.debug(TAG, 'createCommonEventSubscriber ok');
      this.commonEventSubscriber = commonEventSubscriber;
      this.subscribe();
    }).catch((err: Base.BusinessError) => {
      console.error(TAG, `createCommonEventSubscriber fail:${JSON.stringify(err)}}`);
    });
    console.debug(TAG, 'cancelAuthorization end===>');
  }

  subscribe(): void {
    console.debug(TAG, 'subscribe begin===>');
    if(this.commonEventSubscriber === null) {
      console.debug(TAG, 'subscribe commonEventSubscriber is null');
      return;
    }
    CommonEventManager.subscribe(this.commonEventSubscriber, (err: Base.BusinessError,
                                                              data: CommonEventManager.CommonEventData) => {
      if (err.code) {
        console.error(TAG, `commonEventSubscribeCallBack err:${JSON.stringify(err)}`);
        return;
      }
      console.error(TAG, `commonEventSubscribeCallBack data:${JSON.stringify(data)}`);
      let code: number = data.code;
      if (code !== 0) {
        console.error(TAG, 'commonEventSubscribeCallBack !==0');
        return;
      }
      if ('noticeId' in data.parameters) {
        console.debug(TAG, `commonEventSubscribeCallBack data noticeId has value`);
        if (data.parameters.noticeId === NOTICE_ID) {
          console.debug(TAG, `commonEventSubscribeCallBack data event: ${data.event} close notice`);
          injectNoticeUtil.cancelAuthorization();
          injectNoticeUtil.cancelNotificationById(NOTICE_ID);
          return;
        }
      }
    });
    console.debug(TAG, 'subscribe end===>');
  }
}

export let injectNoticeUtil = new InjectNoticeUtil();