#pragma once

#include "Core/StringCrc.h"
#include "Math/Box.hpp"
#include "Math/Matrix.hpp"
#include "Math/Ray.hpp"
#include "TransformComponent.h"

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

	void Build();

	void FrameAll(const cd::AABB& aabb);
	cd::Ray EmitRay(float screenX, float screenY, float width, float height) const;
	// Projection
	const cd::Matrix4x4& GetProjectionMatrix() const { return m_projectionMatrix; }

	void SetAspect(float aspect) { m_aspect = aspect; m_isProjectionDirty = true; }
	void SetAspect(uint16_t width, uint16_t height) { SetAspect(static_cast<float>(width) / static_cast<float>(height)); }
	float GetAspect() const { return m_aspect; }

	void SetFov(float fov) { m_fov = fov; m_isProjectionDirty = true; }
	float GetFov() const { return m_fov; }

	void SetNearPlane(float nearPlane) { m_nearPlane = nearPlane; m_isProjectionDirty = true; }
	float GetNearPlane() const { return m_nearPlane; }

	void SetFarPlane(float farPlane) { m_farPlane = farPlane; m_isProjectionDirty = true; }
	float GetFarPlane() const { return m_farPlane; }

	void SetNDCDepth(cd::NDCDepth ndcDepth) { m_ndcDepth = ndcDepth; m_isProjectionDirty = true; }
	cd::NDCDepth GetNDCDepth() const { return m_ndcDepth; }

	// View
	const cd::Matrix4x4& GetViewMatrix() const { return m_viewMatrix; }

	void SetEye(cd::Point eye) { m_eye = cd::MoveTemp(eye); m_isViewDirty = true; }
	const cd::Point& GetEye() const { return m_eye; }

	void SetLookAt(cd::Direction lookAt) { m_lookAt = cd::MoveTemp(lookAt); m_isViewDirty = true; }
	const cd::Direction& GetLookAt() const { return m_lookAt; }

	void SetUp(cd::Direction up) { m_up = cd::MoveTemp(up); m_isViewDirty = true; }
	const cd::Direction& GetUp() const { return m_up; }

	const cd::Direction& GetCross() const { return m_cross; }

#ifdef EDITOR_MODE
	bool* GetDoConstrainAspectRatio() { return &m_doConstainAspectRatio; }
	bool DoConstrainAspectRatio() const { return m_doConstainAspectRatio; }
	void SetConstrainAspectRatio(bool use) { m_doConstainAspectRatio = use; }

	bool* GetIsPostProcessEnable() { return &m_enablePostProcess; }
	bool IsPostProcessEnable() { return m_enablePostProcess; }
	void SetPostProcessEnable(bool use) { m_enablePostProcess = use; }

	cd::Vec4f& GetGammaCorrection() { return m_gammaCorrection; }
	const cd::Vec4f& GetGammaCorrection() const { return m_gammaCorrection; }
	void SetGammaCorrection(cd::Vec4f gamma) { m_gammaCorrection = cd::MoveTemp(gamma); }
#endif

private:
	// Input
	float m_aspect;
	float m_fov;
	float m_nearPlane;
	float m_farPlane;
	cd::NDCDepth m_ndcDepth;
	cd::Point m_eye;
	cd::Direction m_lookAt;
	cd::Direction m_up;

	// Status
	mutable bool m_isViewDirty;
	mutable bool m_isProjectionDirty;

	// Output
	cd::Direction m_cross;
	cd::Matrix4x4 m_viewMatrix;
	cd::Matrix4x4 m_projectionMatrix;

#ifdef EDITOR_MODE
	bool m_doConstainAspectRatio;
	bool m_enablePostProcess;
	cd::Vec4f m_gammaCorrection;
#endif
};

}