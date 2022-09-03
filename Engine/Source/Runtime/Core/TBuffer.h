#pragma once
#include <stdint.h>

namespace engine::Core
{
    // Buffer with arbitrary data, be careful when use this class
    class TBuffer
    {
    public:
        TBuffer(size_t initSize = 256, size_t minExpandSize = 4096);
        TBuffer(const TBuffer& rhs);
        ~TBuffer();

        template<typename T>
        void PushBack(const T& val);

        // When one data at pos is read, the pos will advance to a new point for next read
        // Note there is no check if the object at pos is T type
        // Do not try to access data out of range
        template<typename T>
        T& GetByPosition(size_t& pos);

        inline size_t GetSize() const { return mSize; }
        inline size_t GetCapacity() const { return mCapacity; }

        inline void Clear() { mSize = 0; };

    private:
        static inline size_t Align(size_t size, size_t align) { return (size + align - 1) & ~(align - 1); }

        void Expand(size_t expandSize);

        // Get available position can write, if space is not enough, expand
        void* GetWritePointer(size_t size, size_t align);

        // Use approximate position to get read pointer, can use output pos as the next read point
        void* GetReadPointer(size_t& pos, size_t size, size_t align);

    private:
        // Init size of TBuffer
        size_t mInitSize;
        // When expand, will choose the max of mMinExpandSize and current capacity
        size_t mMinExpandSize;
        // Current size
        size_t mSize;
        // Current capacity
        size_t mCapacity;
        // Buffer data
        uint8_t* mBuffer;
    };

    template<typename T>
    void TBuffer::PushBack(const T& val)
    {
        void* ptr = GetWritePointer(sizeof(T), alignof(T));
        new(ptr)T(val);
    }

    template<typename T>
    T& TBuffer::GetByPosition(size_t& pos)
    {
        void* ptr = GetReadPointer(pos, sizeof(T), alignof(T));
        return *reinterpret_cast<T*>(ptr);
    }
}
