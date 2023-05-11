#pragma once

#include "Base/Template.h"

namespace engine
{

DEFINE_ENUM_WITH_NAMES(GraphicsBackend, Noop, OpenGLES, OpenGL, Direct3D9, Direct3D11, Direct3D12, Metal, Vulkan)

}