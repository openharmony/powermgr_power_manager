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

import ServiceExtensionAbility from '@ohos.app.ability.ServiceExtensionAbility';
import window from '@ohos.window';
import display from '@ohos.display';

const BG_COLOR = '#00000000';
var g_batteryWindowFirst = undefined;

export default class BatteryServiceExtAbility extends ServiceExtensionAbility {
    /**
     * Lifecycle function, called back when a service extension is started for initialization.
     */
    onCreate(want) {
        console.log('BatteryServiceExtAbility onCreate' + want.abilityName);
        globalThis.extensionContext = this.context;
        globalThis.g_batteryWindowFirst = g_batteryWindowFirst;
    }

    /**
     * Lifecycle function, called back when a service extension is started or recall.
     */
    onRequest(want, startId) {
        globalThis.abilityWant = want;
        console.log('BatteryServiceExtAbility onRequest. start id is ' + startId);
        console.log('want: ' + JSON.stringify(want));
        display.getDefaultDisplay().then(dis => {
            let navigationBarRect = {
                left: 0,
                top: 0,
                width: dis.width,
                height: dis.height
            };
            this.createWindow("Battery Dialog" + startId, window.WindowType.TYPE_FLOAT, navigationBarRect);
        })
    }

    /**
     * Lifecycle function, called back before a service extension is destroyed.
     */
    onDestroy() {
        console.log('BatteryServiceExtAbility onDestroy.');
    }

    private async createWindow(name: string, windowType: number, rect) {
        try {
            if (globalThis.g_batteryWindowFirst != undefined) {
                console.log('destroy first battery window');
                globalThis.g_batteryWindowFirst.destroy();
                globalThis.g_batteryWindowFirst = undefined;
            }
            const batteryWin = await window.create(globalThis.extensionContext, name, windowType);
            if (globalThis.g_batteryWindowFirst == undefined) {
                g_batteryWindowFirst = batteryWin;
                globalThis.g_batteryWindowFirst = g_batteryWindowFirst;
            }
            globalThis.batteryWindow = batteryWin;
            await batteryWin.moveTo(rect.left, rect.top);
            await batteryWin.resetSize(rect.width, rect.height);
            await batteryWin.loadContent('pages/batteryDialog');
            await batteryWin.setBackgroundColor(BG_COLOR);
            await batteryWin.show();
            console.log('Battery window create success');
        } catch {
            console.log('Battery window create failed');
        }
    }
}