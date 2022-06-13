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

import router from '@ohos.router'

export default {
    data: {
        powerOptions: router.getParams().powerOptions,
        shutdownButton: router.getParams().shutdownButton,
        rebootButton: router.getParams().rebootButton,
        cancelButton: router.getParams().cancelButton,
    },
    onInit() {
        this.powerOptions = this.$t("message.powerOptions")
        this.shutdownButton = this.$t("message.shutdownButton")
        this.rebootButton = this.$t("message.rebootButton")
        this.cancelButton = this.$t("message.cancelButton")
    },
    onShutdown() {
        console.info('click shutdown');
        callNativeHandler("EVENT_SHUTDOWN", "shutdown");
    },
    onReboot() {
        console.info('click reboot');
        callNativeHandler("EVENT_REBOOT", "reboot");
    },
    onCancel() {
        console.info('click cancel');
        callNativeHandler("EVENT_CANCEL", "cancel");
    }
}
