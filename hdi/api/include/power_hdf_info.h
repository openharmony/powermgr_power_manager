/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWER_HDF_INFO_H
#define POWER_HDF_INFO_H

enum PowerHdfCmd {
    CMD_SUSPEND = 0,
    CMD_READ_WAKE_COUNT,
    CMD_WRITE_WAKE_COUNT,
    CMD_WAKE_LOCK,
    CMD_WAKE_UNLOCK,
    CMD_DUMP,
};

enum PowerHdfState {
    AWAKE = 0,
    INACTIVE,
    SLEEP,
};

#endif // POWER_HDF_INFO_H