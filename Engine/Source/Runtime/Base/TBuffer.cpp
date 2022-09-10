#include "TBuffer.h"

#include <cassert>
#include <algorithm>

using namespace engine::Core;

TBuffer::TBuffer(size_t initSize, size_t minExpandSize) : mInitSize(initSize), mMinExpandSize(minExpandSize)
{
    mSize = 0;
    mCapacity = initSize;

    mBuffer = (uint8_t*)malloc(mCapacity * sizeof(uint8_t));
}

TBuffer::TBuffer(const TBuffer& rhs) : mInitSize(rhs.mInitSize), mMinExpandSize(rhs.mMinExpandSize)
    , mSize(rhs.mSize)
    , mCapacity(rhs.mCapacity)
{
    mBuffer = (uint8_t*)malloc(mCapacity * sizeof(uint8_t));
    memcpy(mBuffer, rhs.mBuffer, mSize);
}

TBuffer::~TBuffer()
{
    if (mBuffer)
        free(mBuffer);
}

void TBuffer::Expand(size_t size)
{
    size_t expandSize = std::max(size, mMinExpandSize);
    mCapacity += expandSize;

    void* newBufferPtr = std::realloc(mBuffer, mCapacity);
    mBuffer = (uint8_t*)newBufferPtr;
}

void* TBuffer::GetWritePointer(size_t size, size_t align)
{
    size = Align(size, align);
    size_t start = Align(mSize, align);
    size_t end = start + size;
    if (end > mCapacity)
        Expand(size);

    mSize = end;
    assert(mSize <= mCapacity);
    return &mBuffer[start];
}

void* TBuffer::GetReadPointer(size_t& pos, size_t size, size_t align)
{
    size = Align(size, align);
    size_t readPos = Align(pos, align);
    pos = readPos + size;

    assert(pos <= mCapacity);
    pos = readPos + size;
    return &mBuffer[readPos];
}