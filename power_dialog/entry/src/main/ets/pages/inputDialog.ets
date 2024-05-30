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

import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import inputEventClient from '@ohos.multimodalInput.inputEventClient';
import Constants from '../common/constant';

@Extend(Button)
function customizeButton() {
  .backgroundColor(Color.Transparent)
  .fontColor('#0D81F2')
  .fontSize(Constants.BUTTON_TEXT_FONT_SIZE)
  .fontWeight(Constants.BUTTON_TEXT_FONT_WEIGHT)
  .height(Constants.BUTTON_HEIGHT)
  .width(Constants.BUTTON_WIDTH)
}

@CustomDialog
struct InputCustomDialog{
  controller?: CustomDialogController
  cancel: () => void = () => {
  }
  confirm: () => void = () => {
  }
  build() {
    Column() {
      Column() {
        Row() {
          Text($r('app.string.WARNING_TIP'))
            .fontSize(Constants.DIALOG_TITLE_FONT_SIZE)
            .fontColor(Color.Black)
            .fontWeight(Constants.DIALOG_TITLE_FONT_WEIGHT)
            .lineHeight(Constants.DIALOG_TITLE_LINE_HEIGHT)
            .opacity(Constants.DIALOG_TITLE_OPACITY)
        }
      }.margin({
        top: Constants.DIALOG_TITLE_MARGIN_TOP,
        bottom: Constants.DIALOG_TITLE_MARGIN_BOTTOM
      })
      Row(){
        Flex({ justifyContent: FlexAlign.SpaceAround }) {
          Button($r('app.string.BUTTON_CANCEL_POWERDIALOG'))
            .onClick(() => {
              if (this.controller) {
                this.controller.close();
              }
              this.cancel();
              
            })
            .customizeButton()
          Button($r('app.string.BUTTON_CONFIRM'))
            .onClick(() => {
              if (this.controller) {
                this.controller.close();
              }
              this.confirm();
            })
            .customizeButton()
        }.margin({ bottom: 10 })
      }
    }
  }
}

@Entry
@Component
struct InputDialog {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: InputCustomDialog({
      cancel: () => { this.onCancel() },
      confirm: () => { this.onConfirm() },
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: false
  });

  aboutToAppear() {
    console.log('aboutToAppear');
  }
  aboutToDisappear() {
    console.log('aboutToDisappear');
    this.dialogController = null;
  }

  onCancel() {
    try {
      console.log('cancel input');
      inputEventClient.permitInjection(false);
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
