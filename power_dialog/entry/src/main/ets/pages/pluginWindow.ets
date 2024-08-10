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

@Entry
@Component
struct PluginWindow {
  @State titleText: string = ''
  aboutToAppear(): void {
    console.debug('PluginWindow page aboutToAppear');
  }
  
  build() {

    Flex({ alignItems: ItemAlign.Center }) {
      Row() {
        Flex({direction: FlexDirection.Column, justifyContent: FlexAlign.Center}) {
          Text($r('app.string.notice_title'))
            .fontSize($r('sys.float.ohos_id_text_size_headline8'))
            .fontWeight(FontWeight.Medium)
            .align(Alignment.Start)
            .backgroundColor(Color.Orange)

          Text(this.titleText)
            .align(Alignment.Start)
            .fontSize($r('sys.float.ohos_id_text_size_sub_title3'))
            .backgroundColor($r('sys.color.ohos_id_color_text_hint'))
            .margin({
              top: 2
            })
        }
        .padding({
          left: 24
        })
      }
      .flexGrow(1)
      .justifyContent(FlexAlign.Start)
      .padding({ right: $r('sys.float.padding_level2') })


      Column(){
        Image('')
          .width('32vp')
          .height('32vp')
          .backgroundColor(Color.Red)
          .objectFit(ImageFit.Contain)
          .autoResize(false)
          .draggable(false)

      }
      .onClick(() => {
        console.log('1111111111')
      })
      .margin({
        right: 12
      })
    }
    .height(64)
    .width('100%')
    .borderRadius(20)
    .backgroundColor(Color.Gray)
  }

}