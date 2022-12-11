#pragma once

namespace engine
{

class CameraComponent
{
public:
	CameraComponent() = default;
	CameraComponent(const CameraComponent&) = default;
	CameraComponent& operator=(const CameraComponent&) = default;
	CameraComponent(CameraComponent&&) = default;
	CameraComponent& operator=(CameraComponent&&) = default;
	~CameraComponent() = default;
};

}