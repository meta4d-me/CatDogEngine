#pragma once

#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"

namespace engine
{

class ParticleComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("ParticleComponent");
		return className;
	}

	ParticleComponent() = default;
	ParticleComponent(const ParticleComponent&) = default;
	ParticleComponent& operator=(const ParticleComponent&) = default;
	ParticleComponent(ParticleComponent&&) = default;
	ParticleComponent& operator=(ParticleComponent&&) = default;
	~ParticleComponent() = default;

private:

};

}