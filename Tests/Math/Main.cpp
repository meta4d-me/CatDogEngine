#include <cassert>
#include <iostream>

#include "Math/Quaternion.hpp"

// https://quaternions.online/
void TestQuaternion()
{
	cd::Quaternion quaternion(1.0f, 0.0f, 0.0f, 0.0f);
	assert(cd::Quaternion::Identity() == quaternion);
	assert(cd::Quaternion::Identity() == cd::Quaternion::FromPitchYawRoll(0.0f, 0.0f, 0.0f));
	
	float eps = 0.01f;
	{
		// axis x rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(45.0f, 0.0f, 0.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.924f, eps));
		assert(cd::Math::IsEqualTo(q.x(), 0.383f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.0f, eps));
		assert(cd::Math::IsEqualTo(q.z(), 0.0f, eps));
	}

	{
		// axis y rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(0.0f, 45.0f, 0.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.924f, eps));
		assert(cd::Math::IsEqualTo(q.x(), 0.0f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.383f, eps));
		assert(cd::Math::IsEqualTo(q.z(), 0.0f, eps));
	}

	{
		// axis z rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(0.0f, 0.0f, 45.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.924f, eps));
		assert(cd::Math::IsEqualTo(q.x(), 0.0f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.0f, eps));
		assert(cd::Math::IsEqualTo(q.z(), 0.383f, eps));
	}

	{
		// axis x/y rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(45.0f, 45.0f, 0.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.854f, eps));
		assert(cd::Math::IsEqualTo(q.x(), 0.354f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.354f, eps));
		assert(cd::Math::IsEqualTo(q.z(), -0.146f, eps));
	}

	{
		// axis x/z rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(45.0f, 0.0f, 45.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.854f, eps));
		assert(cd::Math::IsEqualTo(q.x(), 0.354f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.146f, eps));
		assert(cd::Math::IsEqualTo(q.z(), 0.354f, eps));
	}

	{
		// axis y/z rotate 45 degrees.
		auto q = cd::Quaternion::FromPitchYawRoll(0.0f, 45.0f, 45.0f);
		assert(cd::Math::IsEqualTo(q.w(), 0.854f, eps));
		assert(cd::Math::IsEqualTo(q.x(), -0.146f, eps));
		assert(cd::Math::IsEqualTo(q.y(), 0.354f, eps));
		assert(cd::Math::IsEqualTo(q.z(), 0.354f, eps));
	}
}

int main()
{
	TestQuaternion();
	return 0;
}