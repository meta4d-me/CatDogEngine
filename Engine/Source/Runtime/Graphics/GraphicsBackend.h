#pragma once

namespace engine
{

enum class GraphicsBackend
{
	Noop,
	OpenGLES,
	OpenGL,
	Direct3D11,
	Direct3D12,
	Metal,
	Vulkan,
	Count
};

}