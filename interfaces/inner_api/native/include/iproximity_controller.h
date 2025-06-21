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

#ifndef POWERMGR_IPROXIMITY_CONTROLLER_H
#define POWERMGR_IPROXIMITY_CONTROLLER_H

namespace OHOS {
namespace PowerMgr {
class IProximityController {
public:
    enum {
        PROXIMITY_AWAY = 0,
        PROXIMITY_CLOSE
    };
    IProximityController() = default;
    virtual ~IProximityController() = default;
    virtual void Enable() = 0;
    virtual void Disable() = 0;
    virtual void OnClose() = 0;
    virtual void OnAway() = 0;

    void SetEnabled(bool enabled)
    {
        enabled_ = enabled;
    }
    bool IsEnabled() const
    {
        return enabled_;
    }
    void SetSupported(bool support)
    {
        support_ = support;
    }
    bool IsSupported() const
    {
        return support_;
    }
    void SetClose(bool isClose)
    {
        isClose_ = isClose;
    }
    bool IsClose() const
    {
        return isClose_;
    }
    void SetStatus(uint32_t status)
    {
        status_ = status;
    }
    uint32_t GetStatus() const
    {
        return status_;
    }
    void Clear()
    {
        isClose_ = false;
    }
private:
    bool enabled_ {false};
    bool support_ {false};
    bool isClose_ {false};
    uint32_t status_ {0};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_IPROXIMITY_CONTROLLER_H