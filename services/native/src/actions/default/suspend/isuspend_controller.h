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

#ifndef POWERMGR_ISUSPEND_CONTROLLER_H
#define POWERMGR_ISUSPEND_CONTROLLER_H

#include "isuspend_controller.h"

namespace OHOS {
namespace PowerMgr {
namespace Suspend {
class ISuspendController {
public:
    using SuspendCallback = std::function<void()>;
    ISuspendController() = default;
    virtual ~ISuspendController() = default;

    virtual void Suspend(SuspendCallback onSuspend, SuspendCallback onWakeup, bool force) = 0;
    virtual void Wakeup();
    virtual void IncSuspendBlockCounter() = 0;
    virtual void DecSuspendBlockCounter() = 0;
};
} // namespace Suspend
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_ISUSPEND_CONTROLLER_H
