#include "Hasher.h"

namespace engine
{


uint32_t Hasher::Hash32(const char* pString)
{
    uint32_t hval = Hash32InitialValue;
    unsigned char* current = (unsigned char*)pString;
    while (*current != 0)
    {
        hval = hval + (hval << 1) + (hval << 4) + (hval << 7) + (hval << 8) + (hval << 24);
        hval = hval ^ (unsigned int)(*current);
        ++current;
    }
    return hval;
}

}