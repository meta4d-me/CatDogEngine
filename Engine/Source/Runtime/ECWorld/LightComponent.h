#pragma once

#include "Core/StringCrc.h"
#include "Rendering/LightUniforms.h"
#include "Scene/LightType.h"
#include <Rendering/RenderTarget.h>
#include <Math/Matrix.hpp>

namespace engine
{

class LightComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("LightComponent");
		return className;
	}

public:
	LightComponent() = default;
	LightComponent(const LightComponent&) = default;
	LightComponent& operator=(const LightComponent&) = default;
	LightComponent(LightComponent&&) = default;
	LightComponent& operator=(LightComponent&&) = default;
	~LightComponent() = default;

	void SetType(cd::LightType type) { m_lightUniformData.type = static_cast<float>(type); }
	cd::LightType GetType() const { return static_cast<cd::LightType>(m_lightUniformData.type); }

	void SetColor(cd::Vec3f color) { m_lightUniformData.color = cd::MoveTemp(color); }
	cd::Vec3f& GetColor() { return m_lightUniformData.color; }
	const cd::Vec3f& GetColor() const { return m_lightUniformData.color; }

	void SetIntensity(float intensity) { m_lightUniformData.intensity = intensity; }
	float& GetIntensity() { return m_lightUniformData.intensity; }
	float GetIntensity() const { return m_lightUniformData.intensity; }

	void SetRange(float range) { m_lightUniformData.range = range; }
	float& GetRange() { return m_lightUniformData.range; }
	float GetRange() const { return m_lightUniformData.range; }

	void SetRadius(float radius) { m_lightUniformData.radius = radius; }
	float& GetRadius() { return m_lightUniformData.radius; }
	float GetRadius() const { return m_lightUniformData.radius; }

	void SetWidth(float width) { m_lightUniformData.width = width; }
	float& GetWidth() { return m_lightUniformData.width; }
	float GetWidth() const { return m_lightUniformData.width; }

	void SetHeight(float height) { m_lightUniformData.height = height; }
	float& GetHeight() { return m_lightUniformData.height; }
	float GetHeight() const { return m_lightUniformData.height; }

	cd::Vec2f GetInnerAndOuter() const;
	void SetInnerAndOuter(float inner, float outer);

	// It is recommended to access the inside and outside angle through GetInnerAndOuter/SetInnerAndOuter.
	void SetAngleScale(float angleScale) { m_lightUniformData.lightAngleScale = angleScale; }
	float& GetAngleScale() { return m_lightUniformData.lightAngleScale; }
	float GetAngleScale() const { return m_lightUniformData.lightAngleScale; }

	// It is recommended to access the inside and outside angle through GetInnerAndOuter/SetInnerAndOuter.
	void SetAngleOffset(float angleOffset) { m_lightUniformData.lightAngleOffeset = angleOffset; }
	float& GetAngleOffset() { return m_lightUniformData.lightAngleOffeset; }
	float GetAngleOffset() const { return m_lightUniformData.lightAngleOffeset; }

	void SetPosition(cd::Point position) { m_lightUniformData.position = cd::MoveTemp(position); }
	cd::Point& GetPosition() { return m_lightUniformData.position; }
	const cd::Point& GetPosition() const { return m_lightUniformData.position; }

	void SetDirection(cd::Direction direction) { m_lightUniformData.direction = cd::MoveTemp(direction); }
	cd::Direction& GetDirection() { return m_lightUniformData.direction; }
	const cd::Direction& GetDirection() const { return m_lightUniformData.direction; }

	void SetUp(cd::Direction up) { m_lightUniformData.up = cd::MoveTemp(up); }
	cd::Direction& GetUp() { return m_lightUniformData.up; }
	const cd::Direction& GetUp() const { return m_lightUniformData.up; }

	U_Light* GetLightUniformData() { return &m_lightUniformData; }

	void SetIsCastShadow(bool isCastShadow) { m_isCastShadow= isCastShadow; };
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
	void ClearShadowMapFB() { for (auto shadowMapFB : m_shadowMapFBs) bgfx::destroy(shadowMapFB); m_shadowMapFBs.clear(); }
	bool IsShadowMapFBsValid() { return m_shadowMapFBs.empty() ? false : true; } //TODO: check all fb's validation
	const std::vector<bgfx::FrameBufferHandle>& GetShadowMapFBs() const { return m_shadowMapFBs; }

	void AddShadowMapTexture(bgfx::TextureHandle& shadowMapTexture) { m_shadowMapTextures.push_back(std::move(shadowMapTexture)); }
	void AddShadowMapTexture(bgfx::TextureHandle&& shadowMapTexture) { m_shadowMapTextures.push_back(shadowMapTexture); }
	void ClearShadowMapTexture() { for (auto shadowMapTexture : m_shadowMapTextures) bgfx::destroy(shadowMapTexture); m_shadowMapTextures.clear(); }
	bool IsShadowMapTexturesValid() { return m_shadowMapTextures.empty() ? false : true; } //TODO: check all fb's validation
	const std::vector<bgfx::TextureHandle>& GetShadowMapTexture() const { return m_shadowMapTextures; }

private:
	U_Light m_lightUniformData;
	bool m_isCastShadow;
	bool m_isCastVolume;
	uint16_t m_shadowMapSize;
	cd::Matrix4x4 m_lightViewProjMatrix;
	std::vector<bgfx::FrameBufferHandle> m_shadowMapFBs;
	std::vector<bgfx::TextureHandle> m_shadowMapTextures;
	// Warning : We treat multiple light components as a complete and contiguous memory.
	// any non-U_Light member of LightComponent will destroy this layout. --2023/6/21
};

}