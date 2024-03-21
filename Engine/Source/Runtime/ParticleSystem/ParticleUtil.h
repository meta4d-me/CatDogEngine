#include "Base/Template.h"
#include "Core/StringCrc.h"
#include "Math/Vector.hpp"

#include <vector>

namespace engine
{
// Catmull-Rom Spline
	cd::Vec3f	GetRibbonPoint(std::vector<cd::Vec3f> points, float t);

}