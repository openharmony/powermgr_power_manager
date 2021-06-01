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

#ifndef POWERMGR_HILOG_WRAPPER_H
#define POWERMGR_HILOG_WRAPPER_H

#define CONFIG_HILOG
#ifdef CONFIG_HILOG
#include "hilog/log.h"
namespace OHOS {
namespace PowerMgr {
#define __FILENAME__            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define __FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__

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

// param of log interface, such as POWER_HILOGF.
enum PowerMgrSubModule {
    MODULE_INNERKIT = 0,
    MODULE_SERVICE,
    MODULE_JAVAKIT, // java kit, defined to avoid repeated use of domain.
    MODULE_JNI,
    MODULE_BATT_INNERKIT,  // below used by battery service
    MODULE_BATT_SERVICE,
    MODULE_BATTERYD,
    MODULE_COMMON,   // used both by battery and powermgr
    MODULE_JS_NAPI,
    POWERMGR_MODULE_BUTT,
};

// 0xD002900: subsystem:PowerMgr module:PowerManager, 8 bits reserved.
static constexpr unsigned int BASE_POWERMGR_DOMAIN_ID = 0xD002900;

enum PowerMgrDomainId {
    POWERMGR_INNERKIT_DOMAIN = BASE_POWERMGR_DOMAIN_ID + MODULE_INNERKIT,
    POWERMGR_SERVICE_DOMAIN,
    POWERMGR_JAVAKIT_DOMAIN,
    BATT_INNERKIT_DOMAIN,
    BATT_SERVICE_DOMAIN,
    BATTERYD_DOMAIN,
    COMMON_DOMAIN,
    POWERMGR_JS_NAPI,
    POWERMGR_BUTT,
};

static constexpr OHOS::HiviewDFX::HiLogLabel POWER_MGR_LABEL[POWERMGR_MODULE_BUTT] = {
    {LOG_CORE, POWERMGR_INNERKIT_DOMAIN, "PowerMgrClient"},
    {LOG_CORE, POWERMGR_SERVICE_DOMAIN, "PowerMgrService"},
    {LOG_CORE, POWERMGR_JAVAKIT_DOMAIN, "PowerMgrJavaService"},
    {LOG_CORE, POWERMGR_INNERKIT_DOMAIN, "PowerMgrJni"},
    {LOG_CORE, BATT_INNERKIT_DOMAIN, "BatterySrvClient"},
    {LOG_CORE, BATT_SERVICE_DOMAIN, "BatteryService"},
    {LOG_CORE, BATTERYD_DOMAIN, "Batteryd"},
    {LOG_CORE, COMMON_DOMAIN, "PowerMgrCommon"},
    {LOG_CORE, POWERMGR_JS_NAPI, "PowerMgrJSNAPI"},
};

// In order to improve performance, do not check the module range.
// Besides, make sure module is less than POWERMGR_MODULE_BUTT.
#define POWER_HILOGF(module, ...) (void)OHOS::HiviewDFX::HiLog::Fatal(POWER_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define POWER_HILOGE(module, ...) (void)OHOS::HiviewDFX::HiLog::Error(POWER_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define POWER_HILOGW(module, ...) (void)OHOS::HiviewDFX::HiLog::Warn(POWER_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define POWER_HILOGI(module, ...) (void)OHOS::HiviewDFX::HiLog::Info(POWER_MGR_LABEL[module], __FORMATED(__VA_ARGS__))
#define POWER_HILOGD(module, ...) (void)OHOS::HiviewDFX::HiLog::Debug(POWER_MGR_LABEL[module], __FORMATED(__VA_ARGS__))

} // namespace PowerMgr
} // namespace OHOS

#else

#define POWER_HILOGF(...)
#define POWER_HILOGE(...)
#define POWER_HILOGW(...)
#define POWER_HILOGI(...)
#define POWER_HILOGD(...)

#endif // CONFIG_HILOG

#endif // POWERMGR_HILOG_WRAPPER_H
