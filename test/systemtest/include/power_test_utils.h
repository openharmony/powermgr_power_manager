/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef POWER_TEST_UTILS_H
#define POWER_TEST_UTILS_H

#include <queue>

namespace OHOS::PowerMgr {
template <typename T>
class DataFactory {
public:
    static const T Consume()
    {
        if (!dataQ_.empty()) {
            const T& data = dataQ_.front();
            dataQ_.pop();
            return data;
        }
        return T{};
    }

    static void Produce(const T& data, int32_t size = 1)
    {
        for (int32_t i = 0; i < size; i++) {
            dataQ_.push(data);
        }
    }

    static void Reset()
    {
        dataQ_ = {};
    }
private:
    static inline std::queue<T> dataQ_ {};
};
} // namespace OHOS::PowerMgr
#endif // POWER_TEST_UTILS_H