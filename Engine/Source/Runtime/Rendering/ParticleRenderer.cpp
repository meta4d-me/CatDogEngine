#include "ParticleRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "RenderContext.h"

#include <filesystem>

#include <bimg/decode.h>
#include <ps/particle_system.h>

namespace engine
{

namespace
{

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat)
{
	size_t size = 0;
	char* data = nullptr;

	std::string fullPath = (std::filesystem::path(CDPROJECT_RESOURCES_ROOT_PATH) / "Textures" / "particle" / _filePath).generic_string();
	std::ifstream file(fullPath, std::ios::binary);
	if (file)
	{
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		data = new char[size];
		file.read(data, size);
	}

	return bimg::imageParse(GetResourceAllocator(), (void*)data, static_cast<uint32_t>(size), bimg::TextureFormat::Enum(_dstFormat));
}

}

static const char* s_shapeNames[] =
{
	"Sphere",
	"Hemisphere",
	"Circle",
	"Disc",
	"Rect",
};

static const char* s_directionName[] =
{
	"Up",
	"Outward",
};

static const char* s_easeFuncName[] =
{
	"Linear",
	"Step",
	"SmoothStep",
	"InQuad",
	"OutQuad",
	"InOutQuad",
	"OutInQuad",
	"InCubic",
	"OutCubic",
	"InOutCubic",
	"OutInCubic",
	"InQuart",
	"OutQuart",
	"InOutQuart",
	"OutInQuart",
	"InQuint",
	"OutQuint",
	"InOutQuint",
	"OutInQuint",
	"InSine",
	"OutSine",
	"InOutSine",
	"OutInSine",
	"InExpo",
	"OutExpo",
	"InOutExpo",
	"OutInExpo",
	"InCirc",
	"OutCirc",
	"InOutCirc",
	"OutInCirc",
	"InElastic",
	"OutElastic",
	"InOutElastic",
	"OutInElastic",
	"InBack",
	"OutBack",
	"InOutBack",
	"OutInBack",
	"InBounce",
	"OutBounce",
	"InOutBounce",
	"OutInBounce",
};

BX_STATIC_ASSERT(BX_COUNTOF(s_easeFuncName) == bx::Easing::Count);

class Emitter
{
public:
	EmitterUniforms m_uniforms;
	EmitterHandle   m_handle;

	EmitterShape::Enum     m_shape;
	EmitterDirection::Enum m_direction;

	void create()
	{
		m_shape = EmitterShape::Sphere;
		m_direction = EmitterDirection::Outward;

		m_handle = psCreateEmitter(m_shape, m_direction, 1024);
		m_uniforms.reset();
	}

	void destroy()
	{
		psDestroyEmitter(m_handle);
	}

	void update()
	{
		psUpdateEmitter(m_handle, &m_uniforms);
	}
};

void ParticleRenderer::Init()
{
	bgfx::setViewName(GetViewID(), "ParticleRenderer");

	psInit();

	bimg::ImageContainer* image = imageLoad(
		"particle.ktx"
		, bgfx::TextureFormat::BGRA8
	);

	EmitterSpriteHandle sprite = psCreateSprite(
		uint16_t(image->m_width)
		, uint16_t(image->m_height)
		, image->m_data
	);

	bimg::imageFree(image);

	m_pEmitter = new Emitter();
	m_pEmitter->create();
	m_pEmitter->m_uniforms.m_handle = sprite;
	m_pEmitter->update();
}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();
}

}