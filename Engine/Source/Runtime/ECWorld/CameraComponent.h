#pragma once

#include "Core/Types.h"
#include "Math/Box.hpp"
#include "Math/Ray.hpp"

namespace cd
{

class Camera;

enum class ToneMappingMode : int
{
	None,
	Exponential,
	Reinhard,
	Reinhard_Luminance,
	Hable_Uncharted2,
	Duiker,
	ACES,
	ACES_Luminance
};

#include "U_ToneMapping.sh"
static_assert(static_cast<int>(ToneMappingMode::None) == TONEMAP_NONE);
static_assert(static_cast<int>(ToneMappingMode::Exponential) == TONEMAP_EXPONENTIAL);
static_assert(static_cast<int>(ToneMappingMode::Reinhard) == TONEMAP_REINHARD);
static_assert(static_cast<int>(ToneMappingMode::Reinhard_Luminance) == TONEMAP_REINHARD_LUM);
static_assert(static_cast<int>(ToneMappingMode::Duiker) == TONEMAP_DUIKER);
static_assert(static_cast<int>(ToneMappingMode::ACES) == TONEMAP_ACES);
static_assert(static_cast<int>(ToneMappingMode::ACES_Luminance) == TONEMAP_ACES_LUM);

}

namespace engine
{

class CameraComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("CameraComponent");
		return className;
	}

public:
	CameraComponent() = default;
	CameraComponent(const CameraComponent&) = default;
	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	~CameraComponent() = default;

	cd::Ray EmitRay(float screenX, float screenY, float width, float height) const;

	void SetAspect(float aspect) { m_aspect = aspect; m_isProjectionDirty = true; }
	void SetAspect(uint16_t width, uint16_t height) { SetAspect(static_cast<float>(width) / static_cast<float>(height)); }
	float& GetAspect() { return m_aspect; }
	float GetAspect() const { return m_aspect; }

	void SetFov(float fov) { m_fov = fov; m_isProjectionDirty = true; }
	float& GetFov() { return m_fov; }
	float GetFov() const { return m_fov; }

	void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; m_isProjectionDirty = true; }
	float& GetNearPlane() { return m_nearPlane; }
	float GetNearPlane() const { return m_nearPlane; }

	void SetFarPlane(float farPlane) { m_farPlane = farPlane; m_isProjectionDirty = true; }
	float& GetFarPlane() { return m_farPlane; }
	float GetFarPlane() const { return m_farPlane; }

	void SetNDCDepth(cd::NDCDepth ndcDepth) { m_ndcDepth = ndcDepth; m_isProjectionDirty = true; }
	cd::NDCDepth GetNDCDepth() const { return m_ndcDepth; }

	// View
	static cd::Vec3f GetLookAt(const cd::Transform& transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 0, 1); }
	static cd::Vec3f GetUp(const cd::Transform& transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0); }
	static cd::Vec3f GetCross(const cd::Transform& transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(1, 0, 0); }
	static void SetLookAt(const cd::Vec3f& lookAt, cd::Transform& transform);
	static void SetUp(const cd::Vec3f& up, cd::Transform& transform);
	static void SetCross(const cd::Vec3f& cross, cd::Transform& transform);
	static void FrameAll(const cd::AABB& aabb, cd::Transform& transform);

	const cd::Matrix4x4& GetViewMatrix() const { return m_viewMatrix; }
	void BuildViewMatrix(const cd::Transform& tranform);
	void BuildViewMatrix(const cd::Vec3f& eye, const cd::Vec3f& lookAt, const cd::Vec3f& up);
	
	// Projection
	const cd::Matrix4x4& GetProjectionMatrix() const { return m_projectionMatrix; }
	void BuildProjectMatrix();

	void Dirty() const { m_isViewDirty = true; m_isProjectionDirty = true; }
	void ViewDirty() const { m_isViewDirty = true; }
	void ProjectDirty() const { m_isProjectionDirty = true; }

#ifdef EDITOR_MODE
	bool& GetDoConstrainAspectRatio() { return m_doConstainAspectRatio; }
	bool DoConstrainAspectRatio() const { return m_doConstainAspectRatio; }
	void SetConstrainAspectRatio(bool use) { m_doConstainAspectRatio = use; }

	// Post Processing
	float& GetExposure() { return m_exposure; }
	float GetExposure() const { return m_exposure; }
	void SetExposure(float exposure) { m_exposure = exposure; }

	float& GetGammaCorrection() { return m_gammaCorrection; }
	float GetGammaCorrection() const { return m_gammaCorrection; }
	void SetGammaCorrection(float gamma) { m_gammaCorrection = gamma; }

	cd::ToneMappingMode& GetToneMappingMode() { return m_toneMappingMode; }
	cd::ToneMappingMode GetToneMappingMode() const { return m_toneMappingMode; }
	void SetToneMappingMode(cd::ToneMappingMode mode) { m_toneMappingMode = mode; }

	// Bloom
	bool& GetIsBloomEnable() { return m_enableBloom; }
	float GetIsBloomEnable() const { return m_enableBloom; }
	void SetBloomEnable(bool bloom) { m_enableBloom = bloom; }

	bool& GetIsBlurEnable() { return m_enableBlur; }
	float GetIsBlurEnable() const { return m_enableBlur; }
	void SetBlurEnable(bool blur) { m_enableBloom = blur; }

	int& GetBloomDownSampleTimes() { return m_bloomDownSampleTimes; }
	int GetBloomDownSampleTimes() const { return m_bloomDownSampleTimes; }
	void SetBloomDownSampleTimes(int times) { m_bloomDownSampleTimes = times; }

	int& GetBloomDownSampleMaxTimes() { return m_bloomDownSampleMaxTimes; }
	int GetBloomDownSampleMaxTimes() const { return m_bloomDownSampleMaxTimes; }
	void SetBloomDownSampleMaxTimes(int maxTimes) { m_bloomDownSampleMaxTimes = maxTimes; }

	float& GetBloomIntensity() { return m_bloomIntensity; }
	float GetBloomIntensity() const { return m_bloomIntensity; }
	void SetBloomIntensity(float intensity) { m_bloomIntensity = intensity; }

	float& GetLuminanceThreshold() { return m_luminanceThreshold; }
	float GetLuminanceThreshold() const { return m_luminanceThreshold; }
	void SetLuminanceThreshold(float luminancethreshold) { m_luminanceThreshold = luminancethreshold; }

	int& GetBlurTimes() { return m_blurTimes; }
	int GetBlurTimes() const { return m_blurTimes; }
	void SetBlurTimes(int blurtimes) { m_blurTimes = blurtimes; }

	int& GetBlurMaxTimes() { return m_blurMaxTimes; }
	int GetBlurMaxTimes() const { return m_blurMaxTimes; }
	void SetBlurMaxTimes(int maxTimes) { m_blurMaxTimes = maxTimes; }

	float& GetBlurSize() { return m_blurSize; }
	float GetBlurSize() const { return m_blurSize; }
	void SetBlurSize(float blursize) { m_blurSize = blursize; }

	int& GetBlurScaling() { return m_blurScaling; }
	int GetBlurScaling() const { return m_blurScaling; }
	void SetBlurScaling(int blurscaling) { m_blurScaling = blurscaling; }
#endif

private:
	// Input
	float m_aspect;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	cd::NDCDepth m_ndcDepth;

	// PostProcessing parameters - pack vec4 as a uniform. Keep order!
	float m_exposure;
	float m_gammaCorrection;
	cd::ToneMappingMode m_toneMappingMode;

	// Status
	mutable bool m_isViewDirty;
	mutable bool m_isProjectionDirty;

	// Output
	cd::Matrix4x4 m_viewMatrix;
	cd::Matrix4x4 m_projectionMatrix;

#ifdef EDITOR_MODE
	bool m_doConstainAspectRatio;
	bool m_enableBloom;
	bool m_enableBlur;
	int m_bloomDownSampleTimes;
	int m_bloomDownSampleMaxTimes;
	float m_bloomIntensity;
	float m_luminanceThreshold;
	int m_blurTimes;
	int m_blurMaxTimes = 20;
	float m_blurSize;
	int m_blurScaling;
#endif
};

}