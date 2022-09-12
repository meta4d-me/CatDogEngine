#pragma once

#include <inttypes.h>

using ObjectGUID = uint64_t;
using ObjectTypeGUID = uint32_t;
using ObjectPropertyUID = uint32_t;
using ObjectListUID = uint32_t;

#ifdef ENGINE_BUILD_SHARED
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API
#endif

#ifdef ENGINE_BUILD_SHARED
#define EDITOR_API __declspec(dllexport)
#else
#define EDITOR_API __declspec(dllimport)
#endif