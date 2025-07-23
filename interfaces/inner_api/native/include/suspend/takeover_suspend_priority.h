/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#ifndef POWERMGR_SUSPEND_PRIORITY_H
#define POWERMGR_SUSPEND_PRIORITY_H

namespace OHOS {
namespace PowerMgr {
/**
 * Suspend or reboot priority
 */
enum class TakeOverSuspendPriority : uint32_t {
    /** Low priority */
    LOW = 0,
    /** Default priority */
    DEFAULT = 1,
    /** High priority */
    HIGH = 2,
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SUSPEND_PRIORITY_H