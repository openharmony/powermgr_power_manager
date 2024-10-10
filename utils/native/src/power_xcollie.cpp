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

#include "power_log.h"
#include "power_xcollie.h"
#include "xcollie/xcollie.h"

namespace OHOS {
namespace PowerMgr {
PowerXCollie::PowerXCollie(const std::string &logTag, bool isRecovery)
{
    logTag_ = logTag;
    isCanceled_ = false;
    const int DFX_DELAY_S = 60;
    unsigned int flag = HiviewDFX::XCOLLIE_FLAG_LOG;
    if (isRecovery) {
        flag = HiviewDFX::XCOLLIE_FLAG_LOG | HiviewDFX::XCOLLIE_FLAG_RECOVERY;
    }
    id_ = HiviewDFX::XCollie::GetInstance().SetTimer(logTag_, DFX_DELAY_S, nullptr, nullptr, flag);
    POWER_HILOGD(FEATURE_UTIL, "Start PowerXCollie, id:%{public}d, tag:%{public}s, timeout(s):%{public}u", id_,
        logTag_.c_str(), DFX_DELAY_S);
}

PowerXCollie::~PowerXCollie()
{
    CancelPowerXCollie();
}

void PowerXCollie::CancelPowerXCollie()
{
    if (!isCanceled_) {
        HiviewDFX::XCollie::GetInstance().CancelTimer(id_);
        isCanceled_ = true;
        POWER_HILOGD(FEATURE_UTIL, "Cancel PowerXCollie, id:%{public}d, tag:%{public}s", id_, logTag_.c_str());
    }
}

} // namespace PowerMgr
} // namespace OHOS