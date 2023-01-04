#pragma once

#include "Core/StringCrc.h"

namespace engine
{

class CameraComponent
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
};

}