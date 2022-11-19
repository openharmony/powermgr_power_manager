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

import Constants from '../common/constant';

@Extend(Button) function customizeButton() {
  .backgroundColor(Color.Transparent)
  .fontColor($r('app.color.button_text_color'))
  .fontSize(Constants.BUTTON_TEXT_FONT_SIZE)
  .fontWeight(Constants.BUTTON_TEXT_FONT_WEIGHT)
  .height(Constants.BUTTON_HEIGHT)
  .width(Constants.BUTTON_WIDTH)
}
@CustomDialog
struct LowCustomDialog {
  controller: CustomDialogController
  cancel: () => void

  build() {
    Column() {
      Column(){
        Row(){
          Text($r('app.string.TEXT_LOW_TEMPERATURE_THERMALDIALOG_LOW'))
            .fontSize(Constants.DIALOG_TITLE_FONT_SIZE)
            .fontColor($r('app.color.title_color'))
            .fontWeight(Constants.DIALOG_TITLE_FONT_WEIGHT)
            .lineHeight(Constants.DIALOG_TITLE_LINE_HEIGHT)
            .opacity(Constants.DIALOG_TITLE_OPACITY)
        }
      }.margin({
        top: Constants.DIALOG_TITLE_MARGIN_TOP,
        bottom: Constants.DIALOG_TITLE_MARGIN_BOTTOM
      })
      Row() {
        Button($r('app.string.BUTTON_KNOW'))
          .onClick(() => {
            this.controller.close()
            this.cancel();
          }).customizeButton()
      }
    }
    .backgroundColor($r('app.color.default_background_color'))
    .borderRadius(Constants.DIALOG_BORDER_RADIUS)
    .width(Constants.DIALOG_WIDTH)
    .height(Constants.DIALOG_HEIGHT)
  }
}

@Entry
@Component
struct LowDialog {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: LowCustomDialog({
      cancel: this.onCancel,
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: true
  })

  onCancel() {
    globalThis.thermalLowWindow.destroy();
    globalThis.g_thermalLowWindowFirst = undefined;
    globalThis.extensionContext.terminateSelf();
  }

  existApp() {
    this.onCancel()
  }

  build() {
    Column(this.dialogController.open()) {}
  }
}
