#pragma once

#include "Core/StringCrc.h"
#include "Rendering/LightUniforms.h"
#include "Scene/LightType.h"
#include <Rendering/RenderTarget.h>
#include <Math/Matrix.hpp>

namespace engine
{

enum class CascadePartitionMode
{
	Manual,
	Logarithmic,
	PSSM,

	Count
};

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

	void SetCascadedNum(uint16_t cascadedSize) { m_cascadedNum = cascadedSize; }
	uint16_t& GetCascadedNum() { return m_cascadedNum; }
	uint16_t GetCascadedNum() const { return m_cascadedNum; }

	void SetCascadePartitionMode(CascadePartitionMode cascadePartitionMode) { m_cascadePartitionMode = cascadePartitionMode; }
	CascadePartitionMode& GetCascadePartitionMode() { return m_cascadePartitionMode; }
	CascadePartitionMode GetCascadePartitionMode() const { return m_cascadePartitionMode; }

	void SetComputedCascadeSplit(float* cascadeSplit) { std::memcpy(&m_computedCascadeSplit[0], cascadeSplit, 16); }
	const float* GetComputedCascadeSplit() { return &m_computedCascadeSplit[0]; }

	float& GetManualCascadeSplitAt(uint16_t idx) { return m_manualCascadeSplit[idx]; }
	const float* GetManualCascadeSplit() { return &m_manualCascadeSplit[0]; }

	void AddLightViewProjMatrix(cd::Matrix4x4 lightViewProjMatrix) { m_lightViewProjMatrices.push_back(lightViewProjMatrix); }
	const std::vector<cd::Matrix4x4>& GetLightViewProjMatrix() { return m_lightViewProjMatrices; }
	const std::vector<cd::Matrix4x4> GetLightViewProjMatrix() const { return m_lightViewProjMatrices; }

	bool IsShadowMapFBsValid();
	void AddShadowMapFB(bgfx::FrameBufferHandle& shadowMapFB) { m_shadowMapFBs.push_back(std::move(shadowMapFB)); }
	const std::vector<bgfx::FrameBufferHandle>& GetShadowMapFBs() const { return m_shadowMapFBs; }
	void ClearShadowMapFBs() { for (auto shadowMapFB : m_shadowMapFBs) bgfx::destroy(shadowMapFB); m_shadowMapFBs.clear(); }
	
	bool IsShadowMapTextureValid() { return bgfx::isValid(m_shadowMapTexture); }
	void SetShadowMapTexture(bgfx::TextureHandle shadowMapTexture) { m_shadowMapTexture = shadowMapTexture; }
	const bgfx::TextureHandle& GetShadowMapTexture() { return m_shadowMapTexture; }
	void ClearShadowMapTexture() { bgfx::destroy(m_shadowMapTexture); m_shadowMapTexture = BGFX_INVALID_HANDLE;}	

	//void Build(); TODO : Need a build function to control switching and deleting lights

private:
	
	U_Light m_lightUniformData;
	bool m_isCastShadow;
	bool m_isCastVolume;
	uint16_t m_shadowMapSize;

	uint16_t m_cascadedNum = 4U;    // dir between [1,4]
	CascadePartitionMode m_cascadePartitionMode = CascadePartitionMode::PSSM;
	float m_manualCascadeSplit[4] = { 0.0 }; // manual set
	float m_computedCascadeSplit[4] = { 0.0 }; // computed

	// uniform
	bgfx::TextureHandle m_shadowMapTexture = BGFX_INVALID_HANDLE;
	std::vector<cd::Matrix4x4> m_lightViewProjMatrices;
	std::vector<bgfx::FrameBufferHandle> m_shadowMapFBs;
	// Warning : We treat multiple light components as a complete and contiguous memory.
	// any non-U_Light member of LightComponent will destroy this layout. --2023/6/21
};

}