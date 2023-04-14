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

#include "parcel.h"

namespace OHOS {

static const size_t DEFAULT_CPACITY = 204800; // 200K

Parcelable::Parcelable() : Parcelable(false) {}

Parcelable::Parcelable(bool asRemote)
{
    asRemote_ = asRemote;
    behavior_ = 0;
}

Parcel::Parcel(Allocator* allocator)
{
    if (allocator != nullptr) {
        allocator_ = allocator;
    } else {
        allocator_ = new DefaultAllocator();
    }

    writeCursor_ = 0;
    readCursor_ = 0;

    data_ = nullptr;
    dataSize_ = 0;
    dataCapacity_ = 0;

    maxDataCapacity_ = DEFAULT_CPACITY;
    objectOffsets_ = nullptr;
    objectCursor_ = 0;
    objectsCapacity_ = 0;
}

Parcel::Parcel() : Parcel(new DefaultAllocator()) {}

Parcel::~Parcel()
{
    FlushBuffer();
    delete allocator_;
}

size_t Parcel::GetWritableBytes() const
{
    return 0;
}

size_t Parcel::GetReadableBytes() const
{
    return 0;
}

size_t Parcel::CalcNewCapacity(size_t minNewCapacity)
{
    (void)minNewCapacity;
    return 0;
}

bool Parcel::EnsureWritableCapacity(size_t desireCapacity)
{
    (void)desireCapacity;
    return false;
}

size_t Parcel::GetDataSize() const
{
    return dataSize_;
}

uintptr_t Parcel::GetData() const
{
    return reinterpret_cast<uintptr_t>(data_);
}

binder_size_t Parcel::GetObjectOffsets() const
{
    return reinterpret_cast<binder_size_t>(objectOffsets_);
}

size_t Parcel::GetOffsetsSize() const
{
    return objectCursor_;
}

size_t Parcel::GetDataCapacity() const
{
    return dataCapacity_;
}

bool Parcel::SetMaxCapacity(size_t maxCapacity)
{
    (void)maxCapacity;
    return false;
}

bool Parcel::SetAllocator(Allocator* allocator)
{
    (void)allocator;
    return false;
}

bool Parcel::CheckOffsets()
{
    return false;
}

void Parcel::InjectOffsets(binder_size_t offsets, size_t offsetSize)
{
    (void)offsets;
    (void)offsetSize;
}

void Parcel::FlushBuffer()
{
    if (allocator_ == nullptr) {
        return;
    }

    if (data_ != nullptr) {
        allocator_->Dealloc(data_);
        dataSize_ = 0;
        writeCursor_ = 0;
        readCursor_ = 0;
        dataCapacity_ = 0;
        data_ = nullptr;
    }

    if (objectOffsets_) {
        objectHolder_.clear();
        free(objectOffsets_);
        objectCursor_ = 0;
        objectOffsets_ = nullptr;
        objectsCapacity_ = 0;
    }
}

bool Parcel::SetDataCapacity(size_t newCapacity)
{
    (void)newCapacity;
    return false;
}

bool Parcel::SetDataSize(size_t dataSize)
{
    (void)dataSize;
    return false;
}

bool Parcel::WriteDataBytes(const void* data, size_t size)
{
    (void)data;
    (void)size;
    return false;
}

void Parcel::WritePadBytes(size_t padSize)
{
    (void)padSize;
}

bool Parcel::WriteBuffer(const void* data, size_t size)
{
    (void)data;
    (void)size;
    return false;
}

bool Parcel::WriteBufferAddTerminator(const void* data, size_t size, size_t typeSize)
{
    (void)data;
    (void)size;
    (void)typeSize;
    return false;
}

bool Parcel::WriteUnpadBuffer(const void* data, size_t size)
{
    return WriteBuffer(data, size);
}

template <typename T>
bool Parcel::Write(T value)
{
    (void)value;
    return false;
}

bool Parcel::WriteBool(bool value)
{
    return Write<int32_t>(static_cast<int32_t>(value));
}

bool Parcel::WriteBoolUnaligned(bool value)
{
    return Write<bool>(value);
}

bool Parcel::WriteInt8(int8_t value)
{
    return Write<int32_t>(static_cast<int32_t>(value));
}

bool Parcel::WriteInt8Unaligned(int8_t value)
{
    return Write<int8_t>(value);
}

bool Parcel::WriteInt16(int16_t value)
{
    return Write<int32_t>(static_cast<int32_t>(value));
}

bool Parcel::WriteInt16Unaligned(int16_t value)
{
    return Write<int16_t>(value);
}

bool Parcel::WriteInt32(int32_t value)
{
    return Write<int32_t>(value);
}

bool Parcel::WriteInt64(int64_t value)
{
    return Write<int64_t>(value);
}

bool Parcel::WriteUint8(uint8_t value)
{
    return Write<uint32_t>(static_cast<uint32_t>(value));
}

bool Parcel::WriteUint8Unaligned(uint8_t value)
{
    return Write<uint8_t>(value);
}

bool Parcel::WriteUint16(uint16_t value)
{
    return Write<uint32_t>(static_cast<uint32_t>(value));
}

bool Parcel::WriteUint16Unaligned(uint16_t value)
{
    return Write<uint16_t>(value);
}

bool Parcel::WriteUint32(uint32_t value)
{
    return Write<uint32_t>(value);
}

bool Parcel::WriteUint64(uint64_t value)
{
    return Write<uint64_t>(value);
}

bool Parcel::WriteFloat(float value)
{
    return Write<float>(value);
}

bool Parcel::WriteDouble(double value)
{
    return Write<double>(value);
}

bool Parcel::WritePointer(uintptr_t value)
{
    return Write<binder_uintptr_t>(value);
}

bool Parcel::WriteCString(const char* value)
{
    (void)value;
    return false;
}

bool Parcel::WriteString(const std::string& value)
{
    (void)value;
    return false;
}

bool Parcel::WriteString16(const std::u16string& value)
{
    (void)value;
    return false;
}

bool Parcel::WriteString16WithLength(const char16_t* value, size_t len)
{
    (void)value;
    (void)len;
    return false;
}

bool Parcel::WriteString8WithLength(const char* value, size_t len)
{
    (void)value;
    (void)len;
    return false;
}

bool Parcel::EnsureObjectsCapacity()
{
    return false;
}

bool Parcel::WriteObjectOffset(binder_size_t offset)
{
    (void)offset;
    return false;
}

bool Parcel::WriteRemoteObject(const Parcelable* object)
{
    (void)object;
    return false;
}

bool Parcel::WriteParcelable(const Parcelable* object)
{
    (void)object;
    return false;
}

bool Parcel::WriteStrongParcelable(const sptr<Parcelable>& object)
{
    (void)object;
    return false;
}

template <typename T>
bool Parcel::Read(T& value)
{
    (void)value;
    return false;
}

template <typename T>
T Parcel::Read()
{
    return 0;
}

bool Parcel::ParseFrom(uintptr_t data, size_t size)
{
    (void)data;
    (void)size;
    return false;
}

const uint8_t* Parcel::ReadBuffer(size_t length)
{
    (void)length;
    return nullptr;
}

const uint8_t* Parcel::ReadUnpadBuffer(size_t length)
{
    (void)length;
    return nullptr;
}

void Parcel::SkipBytes(size_t bytes)
{
    (void)bytes;
}

size_t Parcel::GetReadPosition()
{
    return readCursor_;
}

bool Parcel::RewindRead(size_t newPosition)
{
    (void)newPosition;
    return false;
}

size_t Parcel::GetWritePosition()
{
    return writeCursor_;
}

bool Parcel::RewindWrite(size_t newPosition)
{
    (void)newPosition;
    return false;
}

bool Parcel::ReadBool()
{
    int32_t temp = Read<int32_t>();
    return (temp != 0);
}

bool Parcel::ReadBoolUnaligned()
{
    return Read<bool>();
}

int8_t Parcel::ReadInt8()
{
    int32_t temp = Read<int32_t>();
    return static_cast<int8_t>(temp);
}

int16_t Parcel::ReadInt16()
{
    int32_t temp = Read<int32_t>();
    return static_cast<int16_t>(temp);
}

int32_t Parcel::ReadInt32()
{
    return Read<int32_t>();
}

int64_t Parcel::ReadInt64()
{
    return Read<int64_t>();
}

uint8_t Parcel::ReadUint8()
{
    uint32_t temp = Read<uint32_t>();
    return static_cast<uint8_t>(temp);
}

uint16_t Parcel::ReadUint16()
{
    uint32_t temp = Read<uint32_t>();
    return static_cast<uint16_t>(temp);
}

uint32_t Parcel::ReadUint32()
{
    return Read<uint32_t>();
}

uint64_t Parcel::ReadUint64()
{
    return Read<uint64_t>();
}

float Parcel::ReadFloat()
{
    return Read<float>();
}

double Parcel::ReadDouble()
{
    return Read<double>();
}

template <typename T>
bool Parcel::ReadPadded(T& value)
{
    (void)value;
    return false;
}

bool Parcel::ReadBool(bool& value)
{
    return ReadPadded<bool>(value);
}

bool Parcel::ReadInt8(int8_t& value)
{
    return ReadPadded<int8_t>(value);
}

bool Parcel::ReadInt8Unaligned(int8_t& value)
{
    return Read<int8_t>(value);
}

bool Parcel::ReadInt16(int16_t& value)
{
    return ReadPadded<int16_t>(value);
}

bool Parcel::ReadInt16Unaligned(int16_t& value)
{
    return Read<int16_t>(value);
}

bool Parcel::ReadInt32(int32_t& value)
{
    return Read<int32_t>(value);
}

bool Parcel::ReadInt64(int64_t& value)
{
    return Read<int64_t>(value);
}

bool Parcel::ReadUint8(uint8_t& value)
{
    return ReadPadded<uint8_t>(value);
}

bool Parcel::ReadUint8Unaligned(uint8_t& value)
{
    return Read<uint8_t>(value);
}

bool Parcel::ReadUint16(uint16_t& value)
{
    return ReadPadded<uint16_t>(value);
}

bool Parcel::ReadUint16Unaligned(uint16_t& value)
{
    return Read<uint16_t>(value);
}

bool Parcel::ReadUint32(uint32_t& value)
{
    return Read<uint32_t>(value);
}

bool Parcel::ReadUint64(uint64_t& value)
{
    return Read<uint64_t>(value);
}

bool Parcel::ReadFloat(float& value)
{
    return Read<float>(value);
}

bool Parcel::ReadDouble(double& value)
{
    return Read<double>(value);
}

uintptr_t Parcel::ReadPointer()
{
    return Read<binder_uintptr_t>();
}

const char* Parcel::ReadCString()
{
    return nullptr;
}

const std::string Parcel::ReadString()
{
    return "";
}

bool Parcel::ReadString(std::string& value)
{
    (void)value;
    return false;
}

const std::u16string Parcel::ReadString16()
{
#ifndef MOCK_READSTRING_DESCRIPTIR
    std::u16string descriptor = u"ohos.powermgr.IDisplayPowerCallback";
    return descriptor;
#else
    return std::u16string();
#endif
}

bool Parcel::ReadString16(std::u16string& value)
{
    (void)value;
    return false;
}

const std::u16string Parcel::ReadString16WithLength(int32_t& readLength)
{
    (void)readLength;
    return std::u16string();
}

const std::string Parcel::ReadString8WithLength(int32_t& readLength)
{
    (void)readLength;
    return std::string();
}

template <typename T1, typename T2>
bool Parcel::WriteVector(const std::vector<T1>& val, bool (Parcel::*Write)(T2))
{
    (void)val;
    return false;
}

bool Parcel::WriteBoolVector(const std::vector<bool>& val)
{
    return WriteVector(val, &Parcel::WriteBool);
}

bool Parcel::WriteInt8Vector(const std::vector<int8_t>& val)
{
    return WriteVector(val, &Parcel::WriteInt8Unaligned);
}

bool Parcel::WriteInt16Vector(const std::vector<int16_t>& val)
{
    return WriteVector(val, &Parcel::WriteInt16);
}

bool Parcel::WriteInt32Vector(const std::vector<int32_t>& val)
{
    return WriteVector(val, &Parcel::WriteInt32);
}

bool Parcel::WriteInt64Vector(const std::vector<int64_t>& val)
{
    return WriteVector(val, &Parcel::WriteInt64);
}

bool Parcel::WriteUInt8Vector(const std::vector<uint8_t>& val)
{
    return WriteVector(val, &Parcel::WriteUint8Unaligned);
}

bool Parcel::WriteUInt16Vector(const std::vector<uint16_t>& val)
{
    return WriteVector(val, &Parcel::WriteUint16Unaligned);
}

bool Parcel::WriteUInt32Vector(const std::vector<uint32_t>& val)
{
    return WriteVector(val, &Parcel::WriteUint32);
}

bool Parcel::WriteUInt64Vector(const std::vector<uint64_t>& val)
{
    return WriteVector(val, &Parcel::WriteUint64);
}

bool Parcel::WriteFloatVector(const std::vector<float>& val)
{
    return WriteVector(val, &Parcel::WriteFloat);
}

bool Parcel::WriteDoubleVector(const std::vector<double>& val)
{
    return WriteVector(val, &Parcel::WriteDouble);
}

bool Parcel::WriteStringVector(const std::vector<std::string>& val)
{
    return WriteVector(val, &Parcel::WriteString);
}

bool Parcel::WriteString16Vector(const std::vector<std::u16string>& val)
{
    return WriteVector(val, &Parcel::WriteString16);
}

template <typename T>
bool Parcel::ReadVector(std::vector<T>* val, bool (Parcel::*Read)(T&))
{
    (void)val;
    return false;
}

bool Parcel::ReadBoolVector(std::vector<bool>* val)
{
    (void)val;
    return false;
}

bool Parcel::ReadInt8Vector(std::vector<int8_t>* val)
{
    return ReadVector(val, &Parcel::ReadInt8Unaligned);
}

bool Parcel::ReadInt16Vector(std::vector<int16_t>* val)
{
    return ReadVector(val, &Parcel::ReadInt16);
}

bool Parcel::ReadInt32Vector(std::vector<int32_t>* val)
{
    return ReadVector(val, &Parcel::ReadInt32);
}

bool Parcel::ReadInt64Vector(std::vector<int64_t>* val)
{
    return ReadVector(val, &Parcel::ReadInt64);
}

bool Parcel::ReadUInt8Vector(std::vector<uint8_t>* val)
{
    return ReadVector(val, &Parcel::ReadUint8Unaligned);
}

bool Parcel::ReadUInt16Vector(std::vector<uint16_t>* val)
{
    return ReadVector(val, &Parcel::ReadUint16Unaligned);
}

bool Parcel::ReadUInt32Vector(std::vector<uint32_t>* val)
{
    return ReadVector(val, &Parcel::ReadUint32);
}

bool Parcel::ReadUInt64Vector(std::vector<uint64_t>* val)
{
    return ReadVector(val, &Parcel::ReadUint64);
}

bool Parcel::ReadFloatVector(std::vector<float>* val)
{
    return ReadVector(val, &Parcel::ReadFloat);
}

bool Parcel::ReadDoubleVector(std::vector<double>* val)
{
    return ReadVector(val, &Parcel::ReadDouble);
}

bool Parcel::ReadStringVector(std::vector<std::string>* val)
{
    (void)val;
    return false;
}

bool Parcel::ReadString16Vector(std::vector<std::u16string>* val)
{
    (void)val;
    return false;
}
} // namespace OHOS
