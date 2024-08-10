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

import UIExtensionAbility from '@ohos.app.ability.UIExtensionAbility';
import UIExtensionContentSession from '@ohos.app.ability.UIExtensionContentSession';
import Want from '@ohos.app.ability.Want';
const TAG = 'CapsuleWindowAbility ';

export default class CapsuleWindowAbility extends UIExtensionAbility {
    
    onCreate(): void {
        console.info(TAG, 'onCreate');
    }

    onSessionCreate(want: Want, session: UIExtensionContentSession) {
        console.info(TAG, 'onSessionCreate want:', JSON.stringify(want));
        console.info(TAG, 'onSessionCreate session:', JSON.stringify(session), 'session isNUll:', session == null);
        try {
          let storage: LocalStorage = new LocalStorage({
            'session': session
          });
          session.loadContent('pages/pluginWindow', storage);
        } catch (err) {
            
          console.error(`Failed to load the content. Cause code: ${err}`);
        
      };
      }
    
      onForeground() {
      }
    
      onBackground() {
      }
    
      onSessionDestroy(session: UIExtensionContentSession) {
      }
    
      onDestroy() {
      }
}