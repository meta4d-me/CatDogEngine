#pragma once

#include "Core/StringCrc.h"
#include "Rendering/LightUniforms.h"
#include "Scene/LightType.h"
#include <Rendering/RenderTarget.h>
#include "Math/Transform.hpp"

namespace engine
{

class ShadowComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ShadowComponent");
		return className;
	}

public:
	ShadowComponent() = default;
	ShadowComponent(const ShadowComponent&) = default;
	ShadowComponent& operator=(const ShadowComponent&) = default;
	ShadowComponent(ShadowComponent&&) = default;
	ShadowComponent& operator=(ShadowComponent&&) = default;
	~ShadowComponent() = default;

	bool& GetIsCastShadow() { return m_isCastShadow; }
	bool IsCastShadow() { return m_isCastShadow; }

	bool& GetIsCastVolume() { return m_isCastVolume; }
	bool IsCastVolume() { return m_isCastVolume; }

	void SetShadowMapSize(uint16_t shadowMapSize) { m_shadowMapSize = shadowMapSize; }
	uint16_t& GetShadowMapSize() { return m_shadowMapSize; }
	uint16_t GetShadowMapSize() const { return m_shadowMapSize; }

	void SetLightViewProjMatrix(cd::Matrix4x4 lightViewProjMatrix) { m_lightViewProjMatrix = lightViewProjMatrix; }
	cd::Matrix4x4& GetLightViewProjMatrix() { return m_lightViewProjMatrix; }
	cd::Matrix4x4 GetLightViewProjMatrix() const { return m_lightViewProjMatrix; }

	void AddShadowMapFB(bgfx::FrameBufferHandle& shadowMapFB) { m_shadowMapFBs.push_back(std::move(shadowMapFB)); }
	void ClearShadowMapFB() { for(auto shadowMapFB : m_shadowMapFBs) bgfx::destroy(shadowMapFB); m_shadowMapFBs.clear(); }
	bool IsShadowMapFBsValid() { return m_shadowMapFBs.empty() ? false : true; } //TODO: check all fb's validation
	const std::vector<bgfx::FrameBufferHandle>& GetShadowMapFBs() const { return m_shadowMapFBs; }

	void AddShadowMapTexture(bgfx::TextureHandle& shadowMapTexture) { m_shadowMapTextures.push_back(std::move(shadowMapTexture)); }
	void ClearShadowMapTexture() { for (auto shadowMapTexture : m_shadowMapTextures) bgfx::destroy(shadowMapTexture); m_shadowMapTextures.clear(); }
	bool IsShadowMapTexturesValid() { return m_shadowMapTextures.empty() ? false : true; } //TODO: check all fb's validation
	const std::vector<bgfx::TextureHandle>& GetShadowMapTexture() const { return m_shadowMapTextures; }

private:
	bool m_isCastShadow = true;
	bool m_isCastVolume = true;
	uint16_t m_shadowMapSize;
	cd::Matrix4x4 m_lightViewProjMatrix;
	std::vector<bgfx::FrameBufferHandle> m_shadowMapFBs;
	std::vector<bgfx::TextureHandle> m_shadowMapTextures;
};

}