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
import Base from '@ohos.base';
import type context from '@ohos.app.ability.common';

type Context = context.Context;
const TAG: string = "InjectNotice";
const LABEL: string = "inject notice";

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
  }
  constructor() {

  }

  async init(): Promise<void> {
    console.info(TAG, `init begin`);
    if (this.isInit) {
      return;
    }
    await notificationSubscribe.subscribe(this.subscriber, (err: Base.BusinessError) => { 
      if (err) {
        console.error(TAG + `Failed to subscribe notification. Code is ${err.code}, message is ${err.message}`);
        return;
      }
    });
    this.enableNotification();
    console.info(TAG, `init end`);
    this.isInit = true;
  }

  private onCancelCallBack(data: notificationSubscribe.SubscribeCallbackData): void {
    console.info(TAG, "Cancel callback:" + JSON.stringify(data));
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

  public sendNotice(): boolean {
    console.debug(TAG, 'sendNotice begin=>');
    this.enableNotification();
    console.debug(TAG, 'sendNotice enableNotification end');
    let ret = false;
    try {
      let context: Context = getContext(this) as Context;
      let applicationContext = context.getApplicationContext();
      let imagePixelMap = applicationContext.resourceManager
        .getDrawableDescriptor($r('app.media.icon_notice')).getPixelMap();
    } catch (err) {
      console.error(TAG, `read icon fail: ${JSON.stringify(err)}`);
    }

    let notificationRequest: notificationManager.NotificationRequest = {
      id: 100,
      label: LABEL,
      content: {
        notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_BASIC_TEXT, // 普通文本类型通知
        normal: {
          title: '注入提示',
          text: '注入已经授权',
          additionalText: '注入',
        }

      },

    };
    console.info(TAG, 'publish begin');
    notificationManager.publish(notificationRequest, (err: Base.BusinessError) => {
      if (err) {
        console.error(TAG, TAG + `Failed to publish notification. Code is ${err.code}, message is ${err.message}`);
        return;
      }
      ret = true;
      console.info(TAG, 'Succeeded in publishing notification.');
    });
    return ret;
  }

  async enableNotification(): Promise<void> {

    notificationManager.isNotificationEnabled().then((data: boolean) => {
      console.info(TAG + "isNotificationEnabled success, data: " + JSON.stringify(data));
      if (!data) {
        notificationManager.requestEnableNotification().then(() => {
          console.info(TAG + `[ANS] requestEnableNotification success`);
        }).catch((err: Base.BusinessError) => {
          console.error(TAG + `[ANS] requestEnableNotification failed, code is ${err.code}, message is ${err.message}`);
        });
      }
    }).catch((err: Base.BusinessError) => {
      console.error(TAG + `isNotificationEnabled fail: ${JSON.stringify(err)}`);
    });
  }
};

export let injectNoticeUtil = new InjectNoticeUtil();


