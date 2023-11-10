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

#ifndef SYNC_SLEEP_CALLBACK_IPC_INTERFACE_CODE_H
#define SYNC_SLEEP_CALLBACK_IPC_INTERFACE_CODE_H
namespace OHOS {
namespace PowerMgr {
enum class SyncSleepCallbackInterfaceCode {
    CMD_ON_SYNC_SLEEP = 0,
    CMD_ON_SYNC_WAKEUP = 1,
};
} // namespace PowerMgr
} // namespace OHOS

#endif // SYNC_SLEEP_CALLBACK_IPC_INTERFACE_CODE_H
