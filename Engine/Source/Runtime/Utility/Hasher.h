#pragma once

#include "inttypes.h"

namespace engine
{

class Hasher
{
public:
	static constexpr uint32_t Hash32InitialValue = 0x811c9dc5; // FNV1 initial value
	static uint32_t Hash32(const char* pString);
};

}