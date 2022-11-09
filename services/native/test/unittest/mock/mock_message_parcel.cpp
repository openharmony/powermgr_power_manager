/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "message_parcel.h"

#include <sys/mman.h>
#include <unistd.h>

#include "iremote_object.h"

namespace OHOS {
MessageParcel::MessageParcel() :
    Parcel(), writeRawDataFd_(-1), readRawDataFd_(-1),
    kernelMappedWrite_(nullptr), kernelMappedRead_(nullptr),
    rawData_(nullptr), rawDataSize_(0)
{
}

MessageParcel::MessageParcel(Allocator* allocator) :
    Parcel(allocator), writeRawDataFd_(-1), readRawDataFd_(-1),
    kernelMappedWrite_(nullptr), kernelMappedRead_(nullptr),
    rawData_(nullptr), rawDataSize_(0)
{
}

MessageParcel::~MessageParcel()
{
    if (kernelMappedWrite_ != nullptr) {
        ::munmap(kernelMappedWrite_, rawDataSize_);
        kernelMappedWrite_ = nullptr;
    }
    if (kernelMappedRead_ != nullptr) {
        ::munmap(kernelMappedRead_, rawDataSize_);
        kernelMappedRead_ = nullptr;
    }

    if (readRawDataFd_ > 0) {
        ::close(readRawDataFd_);
        readRawDataFd_ = -1;
    }
    if (writeRawDataFd_ > 0) {
        ::close(writeRawDataFd_);
        writeRawDataFd_ = -1;
    }

    ClearFileDescriptor();

    rawData_ = nullptr;
    rawDataSize_ = 0;
}

#ifndef CONFIG_IPC_SINGLE
bool MessageParcel::WriteDBinderProxy(const sptr<IRemoteObject>& object, uint32_t handle, uint64_t stubIndex)
{
    (void)object;
    (void)handle;
    (void)stubIndex;
    return false;
}
#endif

bool MessageParcel::WriteRemoteObject(const sptr<IRemoteObject>& object)
{
    (void)object;
    return false;
}

sptr<IRemoteObject> MessageParcel::ReadRemoteObject()
{
    return nullptr;
}

bool MessageParcel::WriteFileDescriptor(int fd)
{
    (void)fd;
    return false;
}

int MessageParcel::ReadFileDescriptor()
{
    return -1;
}

void MessageParcel::ClearFileDescriptor() {}

bool MessageParcel::ContainFileDescriptors() const
{
    return false;
}

bool MessageParcel::WriteInterfaceToken(std::u16string name)
{
    #ifdef MOCK_WRITE_INTERFACE_TOKEN_RETURN_TRUE
    return true;
    #else
    return false;
    #endif
}

std::u16string MessageParcel::ReadInterfaceToken()
{
    return ReadString16();
}

bool MessageParcel::WriteRawData(const void* data, size_t size)
{
    (void)data;
    (void)size;
    return false;
}

bool MessageParcel::RestoreRawData(std::shared_ptr<char> rawData, size_t size)
{
    (void)rawData;
    (void)size;
    return false;
}

const void* MessageParcel::ReadRawData(size_t size)
{
    (void)size;
    return nullptr;
}

const void* MessageParcel::GetRawData() const
{
    return nullptr;
}

size_t MessageParcel::GetRawDataSize() const
{
    return rawDataSize_;
}

size_t MessageParcel::GetRawDataCapacity() const
{
    return MAX_RAWDATA_SIZE;
}

void MessageParcel::WriteNoException()
{
    WriteInt32(0);
}

int32_t MessageParcel::ReadException()
{
    return ReadInt32();
}

bool MessageParcel::WriteAshmem(sptr<Ashmem> ashmem)
{
    (void)ashmem;
    return false;
}

sptr<Ashmem> MessageParcel::ReadAshmem()
{
    return nullptr;
}

bool MessageParcel::Append(MessageParcel& data)
{
    (void)data;
    return false;
}
} // namespace OHOS
