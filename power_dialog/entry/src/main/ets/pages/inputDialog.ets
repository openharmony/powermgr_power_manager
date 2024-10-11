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
import ConfigurationConstant from '@ohos.app.ability.ConfigurationConstant';
import util from '@ohos.util';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import inputEventClient from '@ohos.multimodalInput.inputEventClient';

const TAG: string = 'InjectNotice';
const BN_WIDTH: Length = '176vp';

@Extend(Button)
function customizeButton(backcolor?: Color | Resource, titleColor?:Color | Resource) {
  .backgroundColor(backcolor === undefined ? Color.Transparent : backcolor)
  .fontColor(titleColor === undefined ? '#0D81F2' : titleColor)
  .fontSize($r('sys.float.Body_L'))
  .height('40vp')
  .width(BN_WIDTH)
  .fontWeight(FontWeight.Medium)
}

@CustomDialog
struct InputCustomDialog {
  @Prop titleTip: string = '';
  @Prop innerLineColor: ResourceColor = 0;
  controller?: CustomDialogController;
  cancel: () => void = () => {
  }
  confirm: () => void = () => {
  }

  @Builder buildIcon() {
    Row() {
      Image($r('app.media.icon_notice'))
        .width('40vp').height('40vp')
        .objectFit(ImageFit.Contain)
        .autoResize(false)
        .draggable(false)
        .borderRadius('20vp')
    }
    .height('64vp')
    .padding({top: '24vp'})
  }

  @Builder buildTitle() {
    Text(this.titleTip)
      .fontSize($r('sys.float.Title_S'))
      .fontColor($r('sys.color.font_primary'))
      .textAlign(TextAlign.Center)
      .fontWeight(FontWeight.Bold)
      .lineHeight('27vp')
      .padding({ left: '8vp', right: '8vp' })
      .constraintSize({minHeight:'56vp'})
  }

  @Builder buildContext() {
    Text($r('app.string.WARNING_TIP'))
      .fontSize($r('sys.float.Body_L'))
      .fontColor($r('sys.color.font_primary'))
      .textAlign(TextAlign.Center)
      .fontWeight(FontWeight.Medium)
      .padding({ left: '8vp', right: '8vp' })
  }

  @Builder buildBnDeny() {
    Button($r('app.string.bn_not_agree'))
      .type(ButtonType.Normal)
      .borderRadius($r('sys.float.padding_level4'))
      .onClick(() => {
        if (this.controller) {
          this.controller.close();
        }
        this.cancel();
      }).customizeButton($r('sys.color.comp_background_tertiary'), $r('sys.color.font_emphasize'))
    }

  @Builder buildBnAgree() {
    Button($r('app.string.bn_agree'), { type: ButtonType.Normal, stateEffect: true })
      .borderRadius($r('sys.float.padding_level4'))
      .fontColor($r('sys.color.font_on_primary'))
      .onClick(() => {
        if (this.controller) {
          this.controller.close();
        }
        this.confirm();
      })
      .width(BN_WIDTH)
  }

  build() {
    Column() {
      this.buildIcon();
      this.buildTitle();
      this.buildContext();
      Row({ space: '16vp' }) {
        this.buildBnDeny();
        this.buildBnAgree();
      }
      .margin({top:'16vp', bottom:'16vp'})
      .height('40vp')
    }
    .padding({ left: '15vp', right: '15vp' })
    .margin(1)
    .outlineWidth(1)
    .outlineColor(this.innerLineColor)
    .borderRadius($r('sys.float.ohos_id_corner_radius_dialog'))
    .backgroundBlurStyle(BlurStyle.COMPONENT_ULTRA_THIN)
  }
}

@Entry
@Component
struct InputDialog {
  private mContext: common.UIExtensionContext | undefined = undefined;
  @State titleTip: string = '';
  @State outBorderColor : ResourceColor = 0;
  @State innerBorderColor : ResourceColor = 0;
  @StorageProp('currentColorMode') @Watch('onColorModeChange')
  currentMode: number = AppStorage.get('currentColorMode') as number;
  dialogController: CustomDialogController = new CustomDialogController({
    builder: InputCustomDialog({
      cancel: () => { this.onCancel() },
      confirm: () => { this.onConfirm() },
      titleTip: this.titleTip,
      innerLineColor: this.innerBorderColor
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    customStyle: false,
    shadow: ShadowStyle.OUTER_FLOATING_MD,
    borderWidth: '1vp',
    borderColor: this.outBorderColor,
    cornerRadius: $r('sys.float.ohos_id_corner_radius_dialog'),
    backgroundColor: Color.Transparent,
    width: '400vp'
  });

  aboutToAppear() {
    console.log(TAG, `aboutToAppear currentMode:${this.currentMode}`);
    let storage = LocalStorage.getShared();
    this.mContext = storage.get<common.UIExtensionContext>('context');
    let tileTipFormat = this.mContext!.resourceManager.getStringSync($r('app.string.text_inject_tip_title'));
    this.titleTip = util.format(tileTipFormat, '');
    this.onColorModeChange();
    console.log(TAG, `aboutToAppear titelTip:${this.titleTip}`);
  }

  aboutToDisappear() {
    console.log(TAG, 'aboutToDisappear');
  }

  onCancel() {
    try {
      console.log(TAG, 'cancel input');
      let storage = LocalStorage.getShared();
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log(TAG, 'cancel terminateSelf session end');
        session.terminateSelf();
      }
      console.log('cancel end');
    } catch (err) {
      console.error(TAG, 'cancel failed:%{public}s', JSON.stringify(err));
    }
  }

  onConfirm() {
    try {
      console.info(TAG, 'confirm input');
      inputEventClient.permitInjection(true);
      let storage = LocalStorage.getShared();
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log(TAG, 'confirm terminateSelf session end');
        session.terminateSelf();
      }
      console.log(TAG, 'confirm end');
    } catch (err) {
      console.error(TAG, 'confirm failed:%{public}s', JSON.stringify(err));
    }
  }

  existApp() {
    try {
      console.log(TAG, 'existApp input');
      let storage = LocalStorage.getShared();
      let session = storage.get<UIExtensionContentSession>('session');
      if (session) {
        console.log('existApp terminateSelf session');
        session.terminateSelf();
      }
    } catch (err) {
      console.error(TAG, 'existApp failed:%{public}s', JSON.stringify(err));
    }
  }

  build() {
    Column(this.dialogController.open()) {
    }
  }

  onColorModeChange(): void {
    if (this.currentMode == ConfigurationConstant.ColorMode.COLOR_MODE_DARK) {
      console.info(TAG, 'color mode is dark,curMode:' + this.currentMode);
      this.outBorderColor = 'rgba(0, 0, 0, 0.4)';
      this.innerBorderColor = 'rgba(255, 255, 255, 0.2)';
    } else {
      console.info('color mode is not dark curMode:' + this.currentMode);
      this.outBorderColor = 'rgba(0, 0, 0, 0.1)';
      this.innerBorderColor = 'rgba(255, 255, 255, 0.2)';
    }
  }
}
