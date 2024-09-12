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

import util from '@ohos.util';
import notificationManager from '@ohos.notificationManager';
import inputEventClient from '@ohos.multimodalInput.inputEventClient';
import WantAgent, { WantAgent as _WantAgent } from '@ohos.wantAgent';
import CommonEventManager from '@ohos.commonEvent';
import Base from '@ohos.base';
import type context from '@ohos.app.ability.common';
import image from '@ohos.multimedia.image';
import GlobalContext from '../common/GlobalContext';

const TAG: string = 'InjectNotice';
const LABEL: string = 'inject notice';
const EVENT_NAME: string = 'event_inject_close_notice';
export const NOTICE_ID: number = 100;

export enum InjectNoticeStatus {
  DEFAULT = 0,
  OPENING = 1,
  SERVER_CLOSE = 2,
  MAN_CLOSE = 3,
};

class InjectNoticeUtil {
  status: InjectNoticeStatus = InjectNoticeStatus.DEFAULT;
  isInit: boolean = false;
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
  capsuleIcon: image.PixelMap | null = null;
  constructor() {

  }

  async init(): Promise<void> {
    console.info(TAG, `init begin`);
    if (this.isInit) {
      return;
    }
    this.initRemovalWantAgent();
    this.createCommonEventSubscriber();
    let systemLiveViewSubscriber: notificationManager.SystemLiveViewSubscriber = {
      onResponse: this.onLiveNoticeResponseCallback,
    };
    console.info(TAG, 'subscribeSystemLiveView begin');
    notificationManager.subscribeSystemLiveView(systemLiveViewSubscriber).then(() => {
      console.info(TAG, 'subscribeSystemLiveView success');
    }).catch((error: Base.BusinessError) => {
      console.error(`subscribeSystemLiveView fail: ${JSON.stringify(error)}`);
    });
    console.info(TAG, 'initCapsuleIcon begin');
    this.initCapsuleIcon();
    console.info(TAG, 'init end');
    this.isInit = true;
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
    console.debug(TAG, `removalWantAgentObj is ${this.removalWantAgentObj}`);
    let applicationContext = (GlobalContext.getContext().getObject('appcontext')) as context.ApplicationContext;
    let resourceManager = applicationContext.resourceManager;
    let imagePixelMap = resourceManager.getDrawableDescriptor($r('app.media.icon_notice')).getPixelMap();
    let capsuleColor = resourceManager.getColorSync($r('sys.color.ohos_id_color_warning'));
    let title = resourceManager.getStringSync($r('app.string.notice_title'));
    let text = resourceManager.getStringSync($r('app.string.notice_text'));
    text = util.format(text, '');
    let noticeText = resourceManager.getStringSync($r('app.string.notice_title'));
    let cancelBnText = resourceManager.getStringSync($r('app.string.bn_notice_cancel'));
    let cancelBnImage = resourceManager.getDrawableDescriptor($r('app.media.link_slash')).getPixelMap();
    let notificationRequest: notificationManager.NotificationRequest = {
      id: NOTICE_ID,
      label: LABEL,
      isOngoing: true,
      isFloatingIcon: true,
      smallIcon: imagePixelMap,
      isUnremovable: false,
      removalWantAgent: this.removalWantAgentObj,
      slotType: notificationManager.SlotType.LIVE_VIEW,
      content: {
        notificationContentType: notificationManager.ContentType.NOTIFICATION_CONTENT_SYSTEM_LIVE_VIEW,
        systemLiveView: {
          title: title,
          text: text,
          typeCode: 1,
          capsule: {
            title: noticeText,
            backgroundColor: String(capsuleColor),
            icon: this.capsuleIcon!

          },
          button: {
            names: [cancelBnText],
            icons: [cancelBnImage],
          },
        },
      },
    };

    console.debug(TAG, `publish content is ${JSON.stringify(notificationRequest)}`);
    notificationManager.publish(notificationRequest, (err: Base.BusinessError) => {
      if (err) {
        console.error(TAG, TAG + `Failed to publish notification. Code is ${err.code}, message is ${err.message}`);
        return;
      }
      console.info(TAG, 'succeeded in publishing notification.');
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

  async cancelAuthorization(): Promise<void> {
    console.debug(TAG, 'cancelAuthorization begin===>');
    try {
      inputEventClient.permitInjection(false);
    } catch (err: Base.BusinessError) {
      console.error(TAG + `cancelAuthorization fail: ${JSON.stringify(err)}`);
    }
    console.debug(TAG, 'cancelAuthorization end===>');
  }

  createCommonEventSubscriber(): void {
    console.debug(TAG, 'createCommonEventSubscriber begin===>');
    if (this.commonEventSubscriber !== null) {
      console.debug(TAG, `commonEventSubscriber has init`);
      return;
    }
    let subscribeInfo: CommonEventManager.CommonEventSubscribeInfo = {
      events: [EVENT_NAME],
      publisherPermission: 'ohos.permission.INJECT_INPUT_EVENT',
    };
    CommonEventManager.createSubscriber(subscribeInfo).then((commonEventSubscriber: CommonEventManager.CommonEventSubscriber) => {
      console.debug(TAG, 'createCommonEventSubscriber ok');
      this.commonEventSubscriber = commonEventSubscriber;
      this.subscribe();
    }).catch((err: Base.BusinessError) => {
      console.error(TAG, `createCommonEventSubscriber fail:${JSON.stringify(err)}}`);
    });
    console.debug(TAG, 'cancelAuthorization end');
  }
 
  subscribe(): void {
    console.debug(TAG, 'subscribe begin');
    if (this.commonEventSubscriber === null) {
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
    console.debug(TAG, 'subscribe end');
  }

  onSystemLiveBttonsResponse(notificationId: number,
    buttonOptions: notificationManager.ButtonOptions): void {
      console.debug(TAG, 'onSystemLiveBttonsResponse:', JSON.stringify(buttonOptions), 'id:' + notificationId);
  }

  onLiveNoticeResponseCallback(notificationId: number, buttonOptions: notificationManager.ButtonOptions): void {
    console.debug(TAG, 'onLiveNoticeResponseCallback enter');
    if (buttonOptions == null) {
      console.error(TAG, 'onLiveNoticeResponseCallback button callback: ' + 'buttonOptions is null');
      return;
    }
    let clickButtonName = buttonOptions.buttonName;
    let applicationContext = (GlobalContext.getContext().getObject('appcontext')) as context.ApplicationContext;
    let resourceManager = applicationContext.resourceManager;
    let locateButtonName = resourceManager.getStringSync($r('app.string.bn_notice_cancel'));
    console.info(TAG, 'onLiveNoticeResponseCallback button callback: ' + clickButtonName + ', notificationId = ' + notificationId);
    if (clickButtonName === locateButtonName) {
      console.debug(TAG, `onLiveNoticeResponseCallback close notice`);
      injectNoticeUtil.cancelAuthorization();
      injectNoticeUtil.cancelNotificationById(NOTICE_ID);
    }
   }
 
   async initCapsuleIcon(): Promise<void> {
    console.debug(TAG, `initCapsuleIcon begin`);
    if (this.capsuleIcon != null) {
      console.debug(TAG, `initCapsuleIcon has init`);
      return;
    }
    let applicationContext = (GlobalContext.getContext().getObject('appcontext')) as context.ApplicationContext;
    let resourceManager = applicationContext.resourceManager;
    let defaultSize: image.Size = {
      'height': 30,
      'width': 30,
    };
    let svgData = resourceManager.getMediaContentSync($r('app.media.capsule_icon34'));
    let opts: image.DecodingOptions = {
      'index': 0,
      'sampleSize': 1,
      'rotate' :0,
      'editable': true,
      'desiredSize': defaultSize,
      'desiredPixelFormat': 3,
    };
    let imageSource = image.createImageSource(svgData.buffer);
    let svImage: image.PixelMap | null = null;
    svImage = await imageSource.createPixelMap(opts);
    this.capsuleIcon = svImage;
    console.debug(TAG, `initCapsuleIcon end vaule: ${this.capsuleIcon}`);
  }
}

export let injectNoticeUtil = new InjectNoticeUtil();