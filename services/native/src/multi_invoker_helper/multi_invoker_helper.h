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
#ifndef OHOS_POWERMGR_MULTI_INVOKER_HELPER_HEADER
#define OHOS_POWERMGR_MULTI_INVOKER_HELPER_HEADER

#include <bitset>
#include <cinttypes>
#include <mutex>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <iremote_object.h>
#include <refbase.h>
namespace OHOS {
namespace PowerMgr {
constexpr size_t MAX_PARAM_NUMBER = 8;
class MultiInvokerHelper : public IRemoteObject::DeathRecipient {
public:
    // convert booleans to bitset, check the parameter number and type
    template <class... Args> static std::bitset<MAX_PARAM_NUMBER> ToBitset(Args... args)
    {
        static_assert(sizeof...(Args) <= MAX_PARAM_NUMBER, "too many parameters");
        static_assert((... && std::is_same_v<std::remove_cv_t<std::remove_reference_t<Args>>, bool>),
            "each of the parameters needs to be boolean");
        std::string bitString = (... + std::to_string(args));
        return std::bitset<MAX_PARAM_NUMBER> {bitString};
    }

private:
    // only allow construction from sptr. Since the this-pointer needs to be passed to function
    // accepting sptr, stack object going out of scope would make that sptr invalid.
    friend sptr<MultiInvokerHelper>;
    MultiInvokerHelper(size_t paramCount, const std::bitset<MAX_PARAM_NUMBER>& defaults,
        const std::function<void(const std::bitset<MAX_PARAM_NUMBER>&)>& onChange)
        : paramCount_(paramCount > MAX_PARAM_NUMBER ? MAX_PARAM_NUMBER : paramCount),
          sum_(paramCount_, 0),
          defaultParam_(defaults),
          onChange_(onChange)
    {
    }
    std::string DumpInner() const;

    class Invoker {
    public:
        Invoker(size_t paramCount, const sptr<IRemoteObject>& remoteObj)
            : paramCount_(paramCount),
              sum_(paramCount_, 0),
              remoteObj_(remoteObj)
        {
        }
        std::bitset<MAX_PARAM_NUMBER> SetValue(pid_t appid, const std::bitset<MAX_PARAM_NUMBER>& input);
        std::bitset<MAX_PARAM_NUMBER> GetResult() const;
        std::vector<uint64_t> GetSum() const;
        std::string Dump() const;

    private:
        static_assert(sizeof(uint64_t) >= sizeof(pid_t));
        const size_t paramCount_;
        std::vector<uint64_t> sum_;
        const wptr<IRemoteObject> remoteObj_;
        std::unordered_map<pid_t, std::bitset<MAX_PARAM_NUMBER>> entries_ {};
        std::bitset<MAX_PARAM_NUMBER> result_ = 0;
    };

    // If it would be used in an ffrt-task, using ffrt::mutex instead
    std::mutex mutex_;
    std::unordered_map<pid_t, Invoker> invokers_;
    std::bitset<MAX_PARAM_NUMBER> result_ {};
    const size_t paramCount_;
    std::vector<uint64_t> sum_;
    const std::bitset<MAX_PARAM_NUMBER> defaultParam_;
    std::function<void(const std::bitset<MAX_PARAM_NUMBER>&)> onChange_;

public:
    std::string Dump();
    std::bitset<MAX_PARAM_NUMBER> GetResult();
    void OnRemoteDied(const wptr<IRemoteObject>& object) override;
    // remember making the parameter to be the same type as the key of the underlying map;
    bool RemoveInvoker(pid_t pid);
    void Set(const sptr<IRemoteObject>& remoteObj, pid_t invokerPid, pid_t appid, std::bitset<MAX_PARAM_NUMBER>& input);
};
} // namespace PowerMgr
} // namespace OHOS

#endif