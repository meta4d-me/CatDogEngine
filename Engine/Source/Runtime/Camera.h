#pragma once

#include <bx/math.h>

namespace engine
{

class Camera
{
public:
	Camera() = default;
	virtual ~Camera() = default;

	Camera(const Camera&) = delete;
	Camera(Camera&&) = delete;
	Camera& operator=(const Camera&) = delete;
	Camera& operator=(Camera&&) = delete;

	void SetAspect(float aspect);
	void SetFov(float fov);
	void SetNearPlane(float nearPlane);
	void SetFarPlane(float farPlane);
	void SetHomogeneousNdc(bool homogeneousNdc);

	const float* GetProjectionMatrix() const;

	virtual void Update();

	// When camera matrix needs to rebuild, we call this status as "dirty".
	// Such as the old-fashion algorithm "Dirty Rect".
	void Dirty() { m_dirty = true; }

protected:
	bool		m_dirty = true;

private:
	float		m_aspect;
	float		m_fov;
	float		m_near;
	float		m_far;
	bool		m_homogeneousNdc;

	// built matrix
	float		m_projectionMatrix[16];
};

}