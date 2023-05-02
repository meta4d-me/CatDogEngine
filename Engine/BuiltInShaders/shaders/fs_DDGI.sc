$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texClassification, 1);
SAMPLER2D(s_texDistance, 2);
SAMPLER2D(s_texIrradiance, 3);
SAMPLER2D(s_texRelocation, 4);

struct DDGIVolume
{
	vec3 origin;
	vec3 probeSpacing;
	vec3 probeCount;
};

vec3 DDGIGetProbeWorldPosition(vec3 probeCoords, DDGIVolume volume)
{
	// Multiply the grid coordinates by the probe spacing.
	vec3 probeGridWorldPosition = probeCoords * volume.probeSpacing;
	// Shift the grid of probes by half of each axis extent to center the volume about its origin.
	vec3 probeGridShift = (volume.probeSpacing * (volume.probeCount - vec3_splat(1.0))) * vec3_splat(0.5);
	// Center the probe grid about the origin.
	vec3 probeWorldPosition = probeGridWorldPosition - probeGridShift + volume.origin;

	return probeWorldPosition;
}

float SignNotZero(float v)
{
	return (v >= 0.0) ? 1.0 : -1.0;
}

vec2 SignNotZero(vec2 v)
{
	return vec2(SignNotZero(v.x), SignNotZero(v.y));
}

// Computes the normalized octahedral direction that corresponds to the given normalized coordinates on the [-1, 1] square.
vec3 DDGIGetOctahedralDirection(vec2 coords)
{
	vec3 direction = vec3(coords.x, coords.y, 1.0 - abs(coords.x) - abs(coords.y));
	if (direction.z < 0.0)
	{
		direction.xy = (1.0 - abs(direction.yx)) * SignNotZero(direction.xy);
	}
	return normalize(direction);
}

// Computes the octant coordinates in the normalized [-1, 1] square, for the given a unit direction vector.
vec2 DDGIGetOctahedralCoordinates(vec3 direction)
{
	float l1norm = abs(direction.x) + abs(direction.y) + abs(direction.z);
	vec2 uv = direction.xy * (1.0 / l1norm);
	if (direction.z < 0.0)
	{
		uv = (1.0 - abs(uv.yx)) * SignNotZero(uv.xy);
	}
	return uv;
}

vec3 SampleAlbedoTexture(vec2 uv) {
	return texture2D(s_texDistance, uv).xyz;
	// return vec3_splat(5.0) * texture2D(s_texIrradiance, uv).xyz;
}

void main()
{
	gl_FragColor = vec4(SampleAlbedoTexture(v_texcoord0), 1.0);
}
