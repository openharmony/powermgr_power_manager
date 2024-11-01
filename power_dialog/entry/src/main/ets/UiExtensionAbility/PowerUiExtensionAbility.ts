/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';

const TAG: string = 'InjectNotice';

export default class PowerUiExtensionAbility extends UIExtensionAbility {
  onSessionCreate(want, session): void {
    AppStorage.setOrCreate('currentColorMode', this.context.config.colorMode);
    console.debug(TAG, 'powerUiExtensionAbility onSessionCreate currentColorMode:', this.context.config.colorMode);
    let storage: LocalStorage = new LocalStorage({
      'session': session,
      'context': this.context
    });

    if ('isInputDlg' in want.parameters) {
      if (want.parameters.isInputDlg === true) {
        session.loadContent('pages/inputDialog', storage);
        session.setWindowBackgroundColor('#00000000');
        return;
      }
    }

    session.loadContent('pages/powerDialog', storage);
    session.setWindowBackgroundColor('#00000000');
  }

  onForeground(): void {
    AppStorage.setOrCreate('currentColorMode', this.context.config.colorMode);
    console.debug(TAG, 'powerUiExtensionAbility  onForeground currentColorMode:', this.context.config.colorMode);
  }
}
