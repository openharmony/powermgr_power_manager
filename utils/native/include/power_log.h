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

#ifndef POWER_LOG_H
#define POWER_LOG_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG

#include "hilog/log.h"

namespace OHOS {
namespace PowerMgr {
#define FILE_NAME           (__builtin_strrchr("/" __FILE__, '/') + 1)
#define FORMATED(fmt, ...) "[%{public}s:%{public}d] %{public}s# " fmt, FILE_NAME, __LINE__, __FUNCTION__, ##__VA_ARGS__

#ifdef POWER_HILOGF
#undef POWER_HILOGF
#endif

#ifdef POWER_HILOGE
#undef POWER_HILOGE
#endif

#ifdef POWER_HILOGW
#undef POWER_HILOGW
#endif

#ifdef POWER_HILOGI
#undef POWER_HILOGI
#endif

#ifdef POWER_HILOGD
#undef POWER_HILOGD
#endif

namespace {
// Power manager reserved domain id range
constexpr unsigned int POWER_DOMAIN_ID_START = 0xD002900;
constexpr unsigned int POWER_DOMAIN_ID_END = POWER_DOMAIN_ID_START + 32;
constexpr unsigned int TEST_DOMAIN_ID = 0xD000F00;
}

enum PowerManagerLogLabel {
    // Component labels, you can add if needed
    COMP_APP = 0,
    COMP_FWK = 1,
    COMP_SVC = 2,
    COMP_HDI = 3,
    COMP_DRV = 4,
    COMP_UTILS = 5,
    // Feature labels, use to mark major features
    FEATURE_WAKEUP,
    FEATURE_SUSPEND,
    FEATURE_RUNNING_LOCK,
    FEATURE_ACTIVITY,
    FEATURE_POWER_STATE,
    FEATURE_POWER_MODE,
    FEATURE_SHUTDOWN,
    FEATURE_INPUT,
    // Test label
    LABEL_TEST,
    // The end of labels, max to the domain id range length 32
    LABEL_END,
};

enum PowerManagerLogDomain {
    DOMAIN_APP = POWER_DOMAIN_ID_START + COMP_APP, // 0xD002900
    DOMAIN_FRAMEWORK, // 0xD002901
    DOMAIN_SERVICE, // 0xD002902
    DOMAIN_HDI, // 0xD002903
    DOMAIN_DRIVER, // 0xD002904
    DOMAIN_UTILS, // 0xD002905
    DOMAIN_FEATURE_WAKEUP,
    DOMAIN_FEATURE_SUSPEND,
    DOMAIN_FEATURE_RUNNING_LOCK,
    DOMAIN_FEATURE_ACTIVITY,
    DOMAIN_FEATURE_POWER_STATE,
    DOMAIN_FEATURE_POWER_MODE,
    DOMAIN_FEATURE_SHUTDOWN,
    DOMAIN_FEATURE_INPUT,
    DOMAIN_TEST = TEST_DOMAIN_ID, // 0xD000F00
    DOMAIN_END = POWER_DOMAIN_ID_END, // Max to 0xD002920, keep the sequence and length same as PowerManagerLogLabel
};

// Keep the sequence and length same as PowerManagerLogDomain
static constexpr OHOS::HiviewDFX::HiLogLabel POWER_LABEL[LABEL_END] = {
    {LOG_CORE, DOMAIN_APP,                  "PowerApp"},
    {LOG_CORE, DOMAIN_FRAMEWORK,            "PowerFwk"},
    {LOG_CORE, DOMAIN_SERVICE,              "PowerSvc"},
    {LOG_CORE, DOMAIN_HDI,                  "PowerHdi"},
    {LOG_CORE, DOMAIN_DRIVER,               "PowerDrv"},
    {LOG_CORE, DOMAIN_UTILS,                "PowerUtils"},
    {LOG_CORE, DOMAIN_FEATURE_WAKEUP,       "PowerWakeup"},
    {LOG_CORE, DOMAIN_FEATURE_SUSPEND,      "PowerSuspend"},
    {LOG_CORE, DOMAIN_FEATURE_RUNNING_LOCK, "PowerRunningLock"},
    {LOG_CORE, DOMAIN_FEATURE_ACTIVITY,     "PowerActivity"},
    {LOG_CORE, DOMAIN_FEATURE_POWER_STATE,  "PowerState"},
    {LOG_CORE, DOMAIN_FEATURE_POWER_MODE,   "PowerMode"},
    {LOG_CORE, DOMAIN_FEATURE_SHUTDOWN,     "PowerShutdown"},
    {LOG_CORE, DOMAIN_FEATURE_INPUT,        "PowerInput"},
    {LOG_CORE, DOMAIN_TEST,                 "PowerTest"},
};

#define POWER_HILOGF(domain, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(POWER_LABEL[domain], FORMATED(__VA_ARGS__))
#define POWER_HILOGE(domain, ...) (void)OHOS::HiviewDFX::HiLog::Error(POWER_LABEL[domain], FORMATED(__VA_ARGS__))
#define POWER_HILOGW(domain, ...) (void)OHOS::HiviewDFX::HiLog::Warn(POWER_LABEL[domain], FORMATED(__VA_ARGS__))
#define POWER_HILOGI(domain, ...) (void)OHOS::HiviewDFX::HiLog::Info(POWER_LABEL[domain], FORMATED(__VA_ARGS__))
#define POWER_HILOGD(domain, ...) (void)OHOS::HiviewDFX::HiLog::Debug(POWER_LABEL[domain], FORMATED(__VA_ARGS__))

} // namespace PowerMgr
} // namespace OHOS

#else

#define POWER_HILOGF(...)
#define POWER_HILOGE(...)
#define POWER_HILOGW(...)
#define POWER_HILOGI(...)
#define POWER_HILOGD(...)

#endif // CONFIG_HILOG

#endif // POWER_LOG_H
