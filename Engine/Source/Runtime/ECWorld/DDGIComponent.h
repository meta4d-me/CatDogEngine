#pragma once

#include "Core/StringCrc.h"
#include "Math/Matrix.hpp"

#include <span>
#include <vector>

namespace engine
{

class DDGIComponent final
{
public:
	static constexpr StringCrc GetClassName() {
		constexpr StringCrc className("DDGIComponent");
		return className;
	}
};

}
