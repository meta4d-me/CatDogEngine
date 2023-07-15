#include "Math/Quaternion.hpp"
#include "Math/Transform.hpp"
#include "Math/Vector.hpp"

namespace engine
{
static cd::Vec3f GetLookAt(cd::Transform transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 0, 1); }
static cd::Vec3f GetUp(cd::Transform transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(0, 1, 0); }
static cd::Vec3f GetCross(cd::Transform transform) { return transform.GetRotation().ToMatrix3x3() * cd::Vec3f(1, 0, 0); }
static void SetLookAt(cd::Vec3f lookAt, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetLookAt(transform).Cross(lookAt.Normalize());
	float rotAngle = std::acos(GetLookAt(transform).Dot(lookAt.Normalize()));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}

static void SetUp(cd::Vec3f up, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetUp(transform).Cross(up.Normalize());
	float rotAngle = std::acos(GetUp(transform).Dot(up.Normalize()));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}

static void SetCross(cd::Vec3f cross, cd::Transform& transform)
{
	cd::Vec3f rotAxis = GetCross(transform).Cross(cross.Normalize());
	float rotAngle = std::acos(GetUp(transform).Dot(cross.Normalize()));
	transform.SetRotation(transform.GetRotation() * cd::Quaternion::FromAxisAngle(rotAxis, rotAngle));
}
}