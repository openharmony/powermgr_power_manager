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

#ifndef POWERMGR_VIBRATOR_SOURCE_PARSER_H
#define POWERMGR_VIBRATOR_SOURCE_PARSER_H

#include <memory>
#include <string>
#include "json/value.h"

namespace OHOS {
namespace PowerMgr {
class VibratorSource {
public:
    static const constexpr char* ENABLE_KEY = "enable";
    static const constexpr char* TYPE_KEY = "type";
    VibratorSource(std::string scene, bool enable, std::string type) : scene_(scene), enable_(enable), type_(type) {};
    VibratorSource() {};
    ~VibratorSource() = default;
    bool IsEnable() const
    {
        return enable_;
    }
    std::string GetType() const
    {
        return type_;
    }
    std::string GetScene() const
    {
        return scene_;
    }
private:
    std::string scene_;
    bool enable_ = false;
    std::string type_;
};

class VibratorSourceParser {
public:
    std::vector<VibratorSource> ParseSources(
        const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath);

private:
    std::vector<VibratorSource> ParseSources(const std::string& config);
    void GetTargetPath(std::string& targetPath,
        const std::string& etcPath, const std::string& vendorPath, const std::string& systemPath);
    void ParseSourcesProc(std::vector<VibratorSource>& sources, Json::Value& valueObj, std::string& key);
};
} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_VIBRATOR_SOURCE_PARSER_H