#include "ParticleRenderer.h"

#include "ECWorld/CameraComponent.h"
#include "ECWorld/SceneWorld.h"
#include "ECWorld/TransformComponent.h"
#include "Particle/ParticleSystem.h"
#include "RenderContext.h"

#include <bimg/decode.h>
#include <filesystem>
#include <fstream>
#include <string>

namespace engine
{

namespace
{

bimg::ImageContainer* imageLoad(const char* _filePath, bgfx::TextureFormat::Enum _dstFormat)
{
	size_t size = 0;
	char* data = nullptr;

	std::string fullPath = (std::filesystem::path(CDPROJECT_RESOURCES_ROOT_PATH) / "Textures" / "particle" / _filePath).generic_string();
	std::ifstream file(fullPath.c_str(), std::ios::in | std::ios::binary);
	if (file)
	{
		file.seekg(0, file.end);
		size = file.tellg();
		file.seekg(0, file.beg);

		data = new char[size];
		file.read(data, size);

		file.close();
	}
	static bx::DefaultAllocator s_allocator;

	return bimg::imageParse(&s_allocator, (void*)data, static_cast<uint32_t>(size), bimg::TextureFormat::Enum(_dstFormat));
}

}

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

	bgfx::setViewName(GetViewID(), "ParticleRenderer");
}

void ParticleRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	UpdateViewRenderTarget();
	bgfx::setViewTransform(GetViewID(), pViewMatrix, pProjectionMatrix);
}

void ParticleRenderer::Render(float deltaTime)
{
	const cd::Matrix4x4& viewMatrix = m_pCurrentSceneWorld->GetCameraComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetViewMatrix();
	const cd::Transform& cameraTransform = m_pCurrentSceneWorld->GetTransformComponent(m_pCurrentSceneWorld->GetMainCameraEntity())->GetTransform();

	m_pEmitter->update();

	float timeScale = 1.0f;
	psUpdate(deltaTime * timeScale);

	bx::Vec3 cameraPos = { 0.0f, 0.0f , 0.0f };
	memcpy(&cameraPos.x, cameraTransform.GetTranslation().Begin(), 3 * sizeof(float));
	psRender(static_cast<uint8_t>(GetViewID()), viewMatrix.Begin(), cameraPos);
}

}