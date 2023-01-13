#pragma once

#include <cstdint>

namespace engine
{

// Entity is a global unique unsigned integer identifier [0, max - 1] in the engine runtime.
using Entity = uint32_t;
static constexpr Entity INVALID_ENTITY = static_cast<uint32_t>(-1);

}