#pragma once

#include "Math/AABB.hpp"
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

namespace engine
{

class Camera
{
public:
	Camera();
	explicit Camera(cd::Vec3f position);
	explicit Camera(cd::Vec3f position, cd::Vec3f forward, cd::Vec3f up);
	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;
	virtual ~Camera() = default;

	void SetAspect(float aspect);
	void SetAspect(uint16_t width, uint16_t height) { SetAspect(static_cast<float>(width) / height); }
	float GetAspect() const { return m_aspect; }

	void SetFov(float fov);
	float GetFov() const { return m_fov; }

	void SetNearPlane(float nearPlane);
	float GetNearPlane() const { return m_nearPlane; }

	void SetFarPlane(float farPlane);
	float GetFarPlane() const { return m_farPlane; }

	void SetHomogeneousNdc(bool homogeneousNdc);

	const cd::Vec3f& GetPosition() const { return m_position; }
	const cd::Vec3f& GetForwardDir() const { return m_forwardDirection; }
	const cd::Vec3f& GetUpDir() const { return m_upDirection; }

	const float* GetViewMatrix() const { return m_viewMatrix.Begin(); }
	const float* GetProjectionMatrix() const { return m_projectionMatrix.Begin(); }

	void Dirty() const { m_dirty = true; }

	void FrameAll(const cd::AABB& aabb);
	void UpdateProjectionMatrix();
	void UpdateViewMatrix();
	void Update();

private:
	float	m_aspect = 1.778f;
	float	m_fov = 45.0f;
	float	m_nearPlane = 0.1f;
	float	m_farPlane = 1000.0f;

	// OpenGL and DirectX have different NDC diffinitions so this flag is about it.
	// It should set after bgfx inited graphics device.
	bool	m_homogeneousNdc = true;

protected:
	cd::Point m_position;
	cd::Direction m_forwardDirection;
	cd::Direction m_upDirection;

	// For status variable like dirty flag, it is recommended to be mutable
	// because it is actually not a data variable, only a flag to notify changes happened.
	// The benefit is that other const methods can keep const even though they will call Dirty().
	mutable bool m_dirty = true;

	// built matrix
	cd::Matrix4x4 m_viewMatrix2;
	cd::Matrix4x4 m_viewMatrix;
	cd::Matrix4x4 m_projectionMatrix;
};

}