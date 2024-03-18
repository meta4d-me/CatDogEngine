#pragma once

#include "Core/Types.h"
#include "Rendering/LightUniforms.h"
#include "Scene/LightType.h"

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

	void SetShadowType(int shadowType) { m_lightUniformData.shadowType = shadowType; }
	int& GetShadowType() { return m_lightUniformData.shadowType; }
	int GetShadowType() const { return m_lightUniformData.shadowType; }

	void SetLightViewProjOffset(int lightViewProjOffset) { m_lightUniformData.lightViewProjOffset = lightViewProjOffset; }
	int GetLightViewProjOffset() const { return m_lightUniformData.lightViewProjOffset; }

	void SetCascadeNum(int cascadeNum) { m_lightUniformData.cascadeNum = cascadeNum; }
	int& GetCascadeNum() { return m_lightUniformData.cascadeNum; }
	int GetCascadeNum() const { return m_lightUniformData.cascadeNum; }

	void SetShadowBias(float shadowBias) { m_lightUniformData.shadowBias = shadowBias; }
	float& GetShadowBias() { return m_lightUniformData.shadowBias; }
	float GetShadowBias() const { return m_lightUniformData.shadowBias; }

	void SetFrustumClips(cd::Vec4f frustumClips) { m_lightUniformData.frustumClips = frustumClips; }
	cd::Vec4f& GetFrustumClips() { return m_lightUniformData.frustumClips; }
	cd::Vec4f GetFrustumClips() const { return m_lightUniformData.frustumClips; }

	U_Light* GetLightUniformData() { return &m_lightUniformData; }

	void SetCastShadowIntensity(float isCastShadow) { m_castShadowIntensity = isCastShadow; };
	float& GetCastShadowIntensity() { return m_castShadowIntensity; }
	float GetCastShadowIntensity() const { return m_castShadowIntensity; }

	bool& GetIsCastVolume() { return m_isCastVolume; }
	bool IsCastVolume() const { return m_isCastVolume; }

	void SetShadowMapSize(uint16_t shadowMapSize) { m_shadowMapSize = shadowMapSize; }
	uint16_t& GetShadowMapSize() { return m_shadowMapSize; }
	uint16_t GetShadowMapSize() const { return m_shadowMapSize; }

	void SetCascadePartitionMode(CascadePartitionMode cascadePartitionMode) { m_cascadePartitionMode = cascadePartitionMode; }
	CascadePartitionMode& GetCascadePartitionMode() { return m_cascadePartitionMode; }
	CascadePartitionMode GetCascadePartitionMode() const { return m_cascadePartitionMode; }

	void SetComputedCascadeSplit(float* cascadeSplit) { std::memcpy(&m_computedCascadeSplit[0], cascadeSplit, 16); }
	const float* GetComputedCascadeSplit() { return &m_computedCascadeSplit[0]; }

	float& GetManualCascadeSplitAt(uint16_t idx) { return m_manualCascadeSplit[idx]; }
	const float* GetManualCascadeSplit() { return &m_manualCascadeSplit[0]; }

	void AddLightViewProjMatrix(cd::Matrix4x4 lightViewProjMatrix) { m_lightViewProjMatrices.push_back(lightViewProjMatrix); }
	std::vector<cd::Matrix4x4>& GetLightViewProjMatrix() { return m_lightViewProjMatrices; }
	const std::vector<cd::Matrix4x4>& GetLightViewProjMatrix() const { return m_lightViewProjMatrices; }
	void ClearLightViewProjMatrix() { m_lightViewProjMatrices.clear(); }

	bool IsShadowMapFBsValid();
	void AddShadowMapFB(uint16_t& shadowMapFB) { m_shadowMapFBs.push_back(std::move(shadowMapFB)); }
	const std::vector<uint16_t>& GetShadowMapFBs() const { return m_shadowMapFBs; }
	void ClearShadowMapFBs();

	bool IsShadowMapTextureValid();
	void SetShadowMapTexture(uint16_t shadowMapTexture) { m_shadowMapTexture = shadowMapTexture; }
	const uint16_t& GetShadowMapTexture() { return m_shadowMapTexture; }
	void ClearShadowMapTexture();

private:
	U_Light m_lightUniformData;

	float m_castShadowIntensity;
	bool m_isCastVolume;
	uint16_t m_shadowMapSize;

	CascadePartitionMode m_cascadePartitionMode = CascadePartitionMode::PSSM;
	float m_manualCascadeSplit[4] = { 0.0 }; // manual set split
	float m_computedCascadeSplit[4] = { 0.0 }; // computed split

	// uniform
	uint16_t m_shadowMapTexture;	// Texture Handle
	std::vector<cd::Matrix4x4> m_lightViewProjMatrices;
	std::vector<uint16_t> m_shadowMapFBs; // Framebuffer Handle

	// Warning : We treat multiple light components as a complete and contiguous memory.
	// any non-U_Light member of LightComponent will destroy this layout. --2023/6/21
};

}