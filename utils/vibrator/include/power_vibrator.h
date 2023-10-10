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

#ifndef POWERMGR_POWER_VIBRATOR_H
#define POWERMGR_POWER_VIBRATOR_H
#include <mutex>
#include <vector>
#include "vibrator_source_parser.h"
namespace OHOS {
namespace PowerMgr {
class PowerVibrator {
public:
    PowerVibrator() {};
    ~PowerVibrator() = default;
    void LoadConfig(const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath);
    void StartVibrator(const std::string& scene);
    static std::shared_ptr<PowerVibrator> GetInstance();

private:
    std::mutex sourcesMutex_;
    std::vector<VibratorSource> sourceList_;
    static std::mutex mutex_;
    static std::shared_ptr<PowerVibrator> instance_;
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_POWER_VIBRATOR__H
