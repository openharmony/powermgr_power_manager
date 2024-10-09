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

import common from '@ohos.app.ability.common';
import util from '@ohos.util';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import inputEventClient from '@ohos.multimodalInput.inputEventClient';

@Extend(Button)
function customizeButton() {
  .backgroundColor(Color.Transparent)
  .fontColor('#0D81F2')
  .fontSize($r('sys.float.ohos_id_text_size_button1'))
  .height('40vp')
  .width('160vp')
}

@CustomDialog
struct InputCustomDialog {
  @Prop titleTip: string = '';
  controller?: CustomDialogController;
  cancel: () => void = () => {
  }
  confirm: () => void = () => {
  }

  build() {
    Column() {
        Row() {
          Image($r('app.media.icon_notice'))
            .width('40vp')
            .height('40vp')
            .objectFit(ImageFit.Contain)
            .autoResize(false)
            .draggable(false)
        }
        .height('64vp')
        .margin({top: '24vp'})

        Row() {
          Text(this.titleTip)
            .fontSize($r('sys.float.ohos_id_text_size_headline8'))
            .fontColor($r('sys.color.ohos_id_color_text_primary'))
            .textAlign(TextAlign.Center)
        }
        .height('56vp')
        .margin({left: '24vp', right: '24vp'})

        Row() {
          Text($r('app.string.WARNING_TIP'))
            .fontSize($r('sys.float.ohos_id_text_size_button1'))
            .fontColor($r('sys.color.ohos_id_color_text_primary'))
            .textAlign(TextAlign.Center)
        }
        .height('22vp')
        .margin({left: '24vp', right: '24vp' })

        Row(){
          
            Button($r('app.string.bn_not_agree'))
              .onClick(() => {
                if (this.controller) {
                  this.controller.close();
                }
                this.cancel();
              }).customizeButton()

            Button($r('app.string.bn_agree'))
              .onClick(() => {
                if (this.controller) {
                  this.controller.close();
                }
                this.confirm();
              }).customizeButton()
        }
        .height('64vp')
        .margin({top: '8vp', left: '16vp', right: '16vp', bottom: '16vp'})
    }
    .width('400vp')
    .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THICK)
  }
}

@Entry
@Component
struct InputDialog {
  private mContext?: common.UIExtensionContext;
  @State titleTip: string = ''; 
  dialogController: CustomDialogController = new CustomDialogController({
    builder: InputCustomDialog({
      cancel: () => { this.onCancel() },
      confirm: () => { this.onConfirm() },
      titleTip: this.titleTip,
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    customStyle: false,
    cornerRadius: $r('sys.float.ohos_id_corner_radius_dialog'),
  });

  aboutToAppear() {
    console.log('aboutToAppear');
    let storage = LocalStorage.GetShared();
    this.mContext = storage.get<common.UIExtensionContext>('context');
    let tileTipFormat = this.mContext.resourceManager.getStringSync($r('app.string.text_inject_tip_title'));
    this.titleTip = util.format(tileTipFormat, '');
    console.log('InjectNotice', `aboutToAppear titelTip:${this.titleTip}`);
  }

  aboutToDisappear() {
    console.log('aboutToDisappear');
  }

  onCancel() {
    try {
      console.log('cancel input');
      let storage = LocalStorage.GetShared()
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log('cancel terminateSelf session end');
        session.terminateSelf();
      }
      console.log('cancel end');
    } catch (err) {
      console.error('cancel failed:%{public}s', JSON.stringify(err));
    }
  }

  onConfirm() {
    try {
      console.info('confirm input');
      inputEventClient.permitInjection(true);
      let storage = LocalStorage.GetShared()
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log('confirm terminateSelf session end');
        session.terminateSelf();
      }
      console.log('confirm end');
    } catch (err) {
      console.error('confirm failed:%{public}s', JSON.stringify(err));
    }
  }

  existApp() {
    try {
      console.log('existApp input');
      let storage = LocalStorage.GetShared()
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log('existApp terminateSelf session');
        session.terminateSelf();
      }
    } catch (err) {
      console.error('existApp failed:%{public}s', JSON.stringify(err));
    }
  }

  build() {
    Column(this.dialogController.open()) {
    }
  }
}
