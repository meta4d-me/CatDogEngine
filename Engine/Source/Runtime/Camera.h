#pragma once

#include <bx/math.h>

namespace engine
{

class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;

	void SetAspect(float aspect);
	void SetFov(float fov);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	void SetHomogeneousNdc(bool homogeneousNdc);

	void SetEyePosition(bx::Vec3 eye);
	void SetLookTargetPosition(bx::Vec3 lookAt);
	void SetUpDirection(bx::Vec3 up);

	const bx::Vec3& GetEyePosition() const { return m_eye; }
	const bx::Vec3& GetLookTargetPosition() const { return m_lookTarget; }
	const bx::Vec3& GetUpDirection() const { return m_up; }
	bx::Vec3& GetEyePositionForWrite() { return m_eye; }
	bx::Vec3& GetLookTargetPositionForWrite() { return m_lookTarget; }
	bx::Vec3& GetUpDirectionForWrite() { return m_up; }

	void Update();
	const float* GetViewMatrix() const;
	const float* GetProjectionMatrix() const;

	// When camera materix needs to rebuild, we call this status as "dirty".
	// Such as the old-fashion algorithm "Dirty Rect".
	void Dirty() { m_dirty = true; }

private:
	float		m_aspect;
	float		m_fov;
	float		m_near;
	float		m_far;
	bool		m_homogeneousNdc;

	bx::Vec3	m_eye					= { 0.0f, 0.0f, 0.0f };
	bx::Vec3	m_lookTarget			= { 0.0f, 0.0f, 0.0f };
	bx::Vec3	m_up					= { 0.0f, 0.0f, 0.0f };

	bool		m_dirty					= true;

	// built matrix
	float		m_viewMatrix[16];
	float		m_projectionMatrix[16];
};

}