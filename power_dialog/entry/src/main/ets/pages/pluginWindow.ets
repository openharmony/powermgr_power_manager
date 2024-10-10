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

import Base from '@ohos.base';
import common from '@ohos.app.ability.common';
import util from '@ohos.util';

import {CapsuleUtil} from '../InjectNotice/CapsuleUtil'

const TAG = 'InjectNotice';

@Extend(Text) function textModify(width: Length,
  fontSize: number | string | Resource,
  fontColor: ResourceColor, align: Alignment) {
  .width(width)
  .fontSize(fontSize)
  .fontColor(fontColor)
  .align(align)
}

@Entry
@Component
struct PluginWindow {
  private mContext ?: common.UIExtensionContext;
  @State titleText: string = '';
  @State bnDisconnect: string = '';
  @State handlePopup: boolean = false;
  @State hoverColor: ResourceColor = 'sys.color.comp_background_primary';
  private bnHotPress: boolean = false;
  aboutToAppear(): void {
    console.debug('PluginWindow page aboutToAppear');
    this.mContext = getContext(this) as common.UIExtensionContext;
    let text = this.mContext.resourceManager.getStringSync($r('app.string.notice_text'));
    this.titleText = util.format(text, '');
    this.bnDisconnect = this.mContext.resourceManager.getStringSync($r('app.string.bn_notice_cancel'));
    console.debug(`PluginWindow page aboutToAppear titleText:${this.titleText} ${this.bnDisconnect}`);
  }

  @Builder buildTitle() {
    Text($r('app.string.notice_title'))
      .textModify('272vp', $r('sys.float.Title_S'), $r('sys.color.font_primary'), Alignment.Start)
      .fontWeight(FontWeight.Bold);
  }

  @Builder buildContent() {
    Text(this.titleText)
      .textModify('272vp', $r('sys.float.Subtitle_S'), $r('sys.color.font_secondary'), Alignment.Start)
      .fontWeight(FontWeight.Regular)
      .margin({ top: '2vp' });
  }

  @Builder buildSymbolGlyph() {
    SymbolGlyph($r('sys.symbol.link_slash'))
      .fontSize('24vp')
      .fontColor([$r('sys.color.icon_primary')])
      .onClick(() => {
        this.handleBnClick();
      });
  }

  build() {
    Row() {
      Column() {
        this.buildTitle();
        this.buildContent();
      }

      Column() {
        this.buildSymbolGlyph();
      }
      .justifyContent(FlexAlign.Center)
      .backgroundColor(this.hoverColor)
      .width('40vp').height('40vp')
      .bindPopup(this.handlePopup, {
        message: this.bnDisconnect,
      })
      .onHover((isHover: boolean, event: HoverEvent) => {
        this.onBnHover(isHover, event);
      })
      .onMouse((event: MouseEvent) => {
        this.onBnMouse(event);
      })
    }
    .margin({top: '8vp', bottom: '4vp', right: '16vp', left: '24vp'});
  }

  onBnHover(isHover: boolean, event: HoverEvent): void {
    if (isHover) {
      if (!this.bnHotPress) {
        this.handlePopup = true;
        this.hoverColor = $r('sys.color.interactive_hover');
      }
    } else {
      this.handlePopup = false;
    }
    this.hoverColor = 'sys.color.comp_background_primary';
  }

  onBnMouse(event: MouseEvent): void {
    if (event.action == MouseAction.Press) {
      this.bnHotPress = true;
      this.hoverColor = $r('sys.color.interactive_pressed');
    } else {
      this.bnHotPress = false;
    }
  }

  handleBnClick() :void {
    console.debug(TAG, 'handleBnClick entry');
    let instance: CapsuleUtil = CapsuleUtil.getInstance();
    try {
      instance.processCapsule(false);
      instance.closePanel();
    } catch (error) {
      let err = error as Base.BusinessError;
      console.error(TAG, `handleBnClick capsule close err:${JSON.stringify(err)}`);
    }
    console.debug(TAG, `handleBnClick capsule close ok`);
    instance.cancelAuthorization();
    console.debug(TAG, 'handleBnClick exit');
  }
}