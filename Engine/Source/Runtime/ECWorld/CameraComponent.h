#pragma once

#include "Core/StringCrc.h"
#include "Math/Box.hpp"
#include "Math/Ray.hpp"
#include "Math/Transform.hpp"

namespace cd
{

class Camera;

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

	bool& GetIsPostProcessEnable() { return m_enablePostProcess; }
	bool IsPostProcessEnable() { return m_enablePostProcess; }
	void SetPostProcessEnable(bool use) { m_enablePostProcess = use; }

	float& GetGammaCorrection() { return m_gammaCorrection; }
	const float& GetGammaCorrection() const { return m_gammaCorrection; }
	void SetGammaCorrection(float gamma) { m_gammaCorrection = gamma; }

	float& GetBloomIntensity() { return m_bloomIntensity; }
	const float& GetBloomIntensity() const { return m_bloomIntensity; }
	void SetBloomIntensity(float bloomintensity) { m_bloomIntensity = bloomintensity; }

	float& GetBloomRange() { return m_bloomRange; }
	const float& GetBloomRange() const { return m_bloomRange; }
	void SetBloomRange(float bloomrange) { m_bloomRange = bloomrange; }
#endif

private:
	// Input
	float m_aspect;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	cd::NDCDepth m_ndcDepth;

	// Status
	mutable bool m_isViewDirty;
	mutable bool m_isProjectionDirty;

	// Output
	cd::Matrix4x4 m_viewMatrix;
	cd::Matrix4x4 m_projectionMatrix;

#ifdef EDITOR_MODE
	bool m_doConstainAspectRatio;
	bool m_enablePostProcess;
	float m_gammaCorrection;
	float m_bloomIntensity;
	float m_bloomRange;
#endif
};

}