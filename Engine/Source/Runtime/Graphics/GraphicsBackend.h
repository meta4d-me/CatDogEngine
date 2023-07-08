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

static constexpr const char* GraphicsBackendNames[] =
{
	"Noop",
	"OpenGLES",
	"OpenGL",
	"Direct3D11",
	"Direct3D12",
	"Metal",
	"Vulkan",
	"Unknown"
};

static_assert(sizeof(GraphicsBackendNames) / sizeof(const char*) - 1 == static_cast<int>(GraphicsBackend::Count));

static constexpr const char* GetGraphicsBackendName(GraphicsBackend backend)
{
	return GraphicsBackendNames[static_cast<int>(backend)];
}

}