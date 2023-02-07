#pragma once

#include "Core/StringCrc.h"

namespace engine
{

class AnimationComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("AnimationComponent");
		return className;
	}

public:
	AnimationComponent() = default;
	AnimationComponent(const AnimationComponent&) = default;
	AnimationComponent& operator=(const AnimationComponent&) = default;
	AnimationComponent(AnimationComponent&&) = default;
	AnimationComponent& operator=(AnimationComponent&&) = default;
	~AnimationComponent() = default;
};

}