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

#include "power_vibrator.h"

#include <string>
#include "power_log.h"
#ifdef POWER_VIBRATOR_MODE_ENABLE
#include "vibrator_agent.h"
#endif
#include "vibrator_source_parser.h"

namespace OHOS {
namespace PowerMgr {
namespace {
#ifdef POWER_VIBRATOR_MODE_ENABLE
const int32_t VIBRATOR_SUCCESS = 0;
#endif
}
std::mutex PowerVibrator::mutex_;
std::shared_ptr<PowerVibrator> PowerVibrator::instance_ = nullptr;

void PowerVibrator::LoadConfig(
    const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath)
{
    std::shared_ptr<VibratorSourceParser> parser = std::make_shared<VibratorSourceParser>();
    std::vector<VibratorSource> sources = parser->ParseSources(etcPath, vendorPath, systemPath);
    std::lock_guard<std::mutex> lock(sourcesMutex_);
    for (auto source : sources) {
        sourceList_.emplace_back(source);
    }
}

void PowerVibrator::StartVibrator(const std::string& scene)
{
#ifdef POWER_VIBRATOR_MODE_ENABLE
    VibratorSource source;
    for (VibratorSource& src : sourceList_) {
        if (src.GetScene() == scene) {
            source = src;
            break;
        }
    }
    if (!source.IsEnable() || source.GetType() == "") {
        POWER_HILOGI(COMP_UTILS, "%{public}s do not need vibrator", scene.c_str());
        return;
    }
    int32_t ret = Sensors::StartVibrator(source.GetType().c_str());
    if (ret != VIBRATOR_SUCCESS) {
        POWER_HILOGE(COMP_UTILS, "%{public}s vibrator failed", scene.c_str());
        return;
    }
#else
    return;
#endif
}

std::shared_ptr<PowerVibrator> PowerVibrator::GetInstance()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = std::make_shared<PowerVibrator>();
    return instance_;
}
} // namespace PowerMgr
} // namespace OHOS
