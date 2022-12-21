#pragma once
#include "Math/VectorDerived.hpp"

namespace engine
{

namespace
{
	using vec3 = cd::Vec3f;
}

struct PointLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
};

struct SpotLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;

	vec3 direction;
	float lightAngleScale;
	float lightAngleOffeset;
};

struct DirectionalLight {
	vec3 color;
	// Irradiance/Illuminance in Luxes
	float intensity;
	vec3 direction;
};

struct SphereLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;

	float radius;
};

struct DiskLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;

	vec3 direction;
	float radius;
};

struct RectangleLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;

	vec3 direction;
	float width;
	vec3 up;
	float height;
};

struct TubeLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;

	vec3 direction;
	float radius;
	float width;
};

} // namespace engine
