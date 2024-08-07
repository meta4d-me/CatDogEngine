#include <vector>

class DataView
{
public:
    DataView(const std::vector<std::byte>& buffer, size_t offset)
        : buffer(buffer), offset(offset)
    {
    }

    template <typename T>
    T get(size_t index, bool littleEndian = true) const
    {
        T value;
        std::memcpy(&value, &buffer[offset + index], sizeof(T));
        if (littleEndian)
        {
            // Convert to little-endian if necessary
            value = toLittleEndian(value);
        }
        return value;
    }

private:
    const std::vector<std::byte>& buffer;
    size_t offset;

    template <typename T>
    T toLittleEndian(T value) const
    {
        // Implement endian conversion if needed
        return value;
    }
};