#pragma once

#include "Math/AABB.hpp"
#include "Math/Matrix.hpp"
#include "Math/Ray.hpp"
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

	const cd::Point& GetPosition() const { return m_position; }
	const cd::Direction& GetForward() const { return m_forwardDirection; }
	const cd::Direction& GetUp() const { return m_upDirection; }

	const cd::Matrix4x4& GetViewMatrix();
	const cd::Matrix4x4& GetProjectionMatrix();

	void Dirty() { m_dirty = true; }

	void FrameAll(const cd::AABB& aabb);
	cd::Ray EmitRay(float screenX, float screenY, float width, float height);

	void UpdateProjectionMatrix();
	void UpdateViewMatrix();

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

	bool m_dirty = true;

	// built matrix
	cd::Matrix4x4 m_viewMatrix;
	cd::Matrix4x4 m_projectionMatrix;
};

}