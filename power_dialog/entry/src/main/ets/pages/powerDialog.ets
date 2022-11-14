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

import power from '@ohos.power';
import hilog from '@ohos.hilog';
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
struct PowerCustomDialog {
  controller: CustomDialogController
  cancel: () => void
  shutdown: () => void
  reboot: () => void

  build() {
    Column() {
      Column() {
        Row() {
          Text($r('app.string.TEXT_POWER_OPTIONS_POWERDIALOG'))
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
        Button($r('app.string.BUTTON_CANCEL_POWERDIALOG'))
          .onClick(() => {
            this.controller.close()
            this.cancel()
          })
          .customizeButton()
          .margin({right: Constants.BUTTON_MARGIN})
        Text('|')
          .fontSize(Constants.SPLIT_FONT_SIZE)
          .fontColor($r('app.color.split_color'))
        Button($r('app.string.BUTTON_REBOOT_POWERDIALOG'))
          .onClick(() => {
            this.reboot()
          })
          .customizeButton()
          .margin({left: Constants.BUTTON_MARGIN, right: Constants.BUTTON_MARGIN})
        Text('|')
          .fontSize(Constants.SPLIT_FONT_SIZE)
          .fontColor($r('app.color.split_color'))
        Button($r('app.string.BUTTON_SHUTDOWN_POWERDIALOG'))
          .onClick(() => {
            this.shutdown()
          })
          .customizeButton()
          .margin({left: Constants.BUTTON_MARGIN})
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
struct PowerDialog {
  dialogController: CustomDialogController = new CustomDialogController({
    builder: PowerCustomDialog({
      cancel: this.onCancel,
      shutdown: this.onShutdown,
      reboot: this.onReboot
    }),
    cancel: this.existApp,
    autoCancel: false,
    alignment: DialogAlignment.Center,
    offset: { dx: 0, dy: -20 },
    gridCount: 4,
    customStyle: true
  })

  onCancel() {
    globalThis.powerWindow.destroy();
    globalThis.g_powerWindowFirst = undefined;
    globalThis.extensionContext.terminateSelf();
  }

  onShutdown() {
    power.shutdown("power_dialog")
  }

  onReboot() {
    power.reboot("power_dialog")
  }

  existApp() {
    this.onCancel()
  }

  build() {
    Column(this.dialogController.open()) {}
  }
}
