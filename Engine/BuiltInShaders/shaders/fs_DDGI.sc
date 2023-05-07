$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

#define DISTANCE_TEXELS_COUNT 16
#define IRRADIANCE_TEXELS_COUNT 8

uniform vec4 u_volumeOrigin;
uniform vec4 u_volumeProbeSpacing;
uniform vec4 u_volumeProbeCounts;

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texClassification, 1);
SAMPLER2D(s_texDistance, 2);
USAMPLER2D(s_texIrradiance, 3);
SAMPLER2D(s_texRelocation, 4);

vec3 SampleIrradiance(vec2 uv)
{
	return vec3(texelFetch(s_texIrradiance, ivec2(uv * ivec2(80, 32)), 0).xyz);
}

vec3 SampleDistance(vec2 uv)
{
	return texture2D(s_texDistance, uv).xyz;
}

struct DDGIVolume
{
	vec3 origin;
	vec3 probeSpacing;
	vec3 probeCounts;
};

DDGIVolume GetVolumeData()
{
	DDGIVolume volume;
	volume.origin = u_volumeOrigin.xyz;
	volume.probeSpacing = u_volumeProbeSpacing.xyz;
	volume.probeCounts = u_volumeProbeCounts.xyz;
	return volume;
}

// Computes the world-space position of a probe from the probe's 3D grid-space coordinates.
// Probe relocation is not considered.
vec3 DDGIGetProbeWorldPosition(vec3 probeCoords, DDGIVolume volume)
{
	// Multiply the grid coordinates by the probe spacing.
	vec3 probeGridWorldPosition = probeCoords * volume.probeSpacing;
	// Shift the grid of probes by half of each axis extent to center the volume about its origin.
	vec3 probeGridShift = (volume.probeSpacing * (volume.probeCounts - vec3_splat(1.0))) * vec3_splat(0.5);
	// Center the probe grid about the origin.
	vec3 probeWorldPosition = probeGridWorldPosition - probeGridShift + volume.origin;

	return probeWorldPosition;
}

// ----- Math ----- //

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

// ----- Probe Indexing ----- //

// Computes the 3D grid-space coordinates of the "base" probe (i.e. floor of xyz) of the 8-probe
// cube that surrounds the given world space position. The other seven probes of the cube
// are offset by 0 or 1 in grid space along each axis.
// This function accounts for scroll offsets to adjust the volume's origin.
ivec3 DDGIGetBaseProbeGridCoords(vec3 worldPosition, DDGIVolume volume)
{
	// Get the vector from the volume origin to the surface point.
	vec3 position = worldPosition - (volume.origin + volume.probeSpacing);
	
	// Shift from [-n/2, n/2] to [0, n] (grid space).
	position += (volume.probeSpacing * (volume.probeCounts - 1)) * 0.5;
	
	// Quantize the position to grid space.
	ivec3 probeCoords = ivec3(position / volume.probeSpacing);
	
	// Clamp to [0, probeCounts - 1].
	// Snaps positions outside of grid to the grid edge.
	probeCoords = clamp(probeCoords, ivec3(0, 0, 0), (volume.probeCounts - ivec3(1, 1, 1)));
	
	return probeCoords;
}


// Get the number of probes on a horizontal plane, in the active coordinate system.
int DDGIGetProbesPerPlane(ivec3 probeCounts)
{
	return (probeCounts.x * probeCounts.z);
}

// Get the index of the horizontal plane, in the active coordinate system.
int DDGIGetPlaneIndex(ivec3 probeCoords)
{
	return probeCoords.y;
}

// Get the index of a probe within a horizontal plane that the probe coordinates map to, in the active coordinate system.
int DDGIGetProbeIndexInPlane(ivec3 probeCoords, ivec3 probeCounts)
{
	return probeCoords.x + (probeCounts.x * probeCoords.z);
}

// Computes the probe index from 3D grid coordinates.
// The opposite of DDGIGetProbeCoords(probeIndex,...).
int DDGIGetProbeIndex(int3 probeCoords, DDGIVolume volume)
{
	int probesPerPlane = DDGIGetProbesPerPlane(volume.probeCounts);
	int planeIndex = DDGIGetPlaneIndex(probeCoords);
	int probeIndexInPlane = DDGIGetProbeIndexInPlane(probeCoords, volume.probeCounts);
	
	return (planeIndex * probesPerPlane) + probeIndexInPlane;
}

// Computes the Texture2DArray coordinates of the probe at the given probe index.
// When infinite scrolling is enabled, probeIndex is expected to be the scroll adjusted probe index.
// Obtain the adjusted index with DDGIGetScrollingProbeIndex().
ivec3 DDGIGetProbeTexelCoords(int probeIndex, DDGIVolume volume)
{
	// Find the probe's plane index.
	int probesPerPlane = DDGIGetProbesPerPlane(volume.probeCounts);
	int planeIndex = int(probeIndex / probesPerPlane);
	
	int x = (probeIndex % volume.probeCounts.x);
	int y = (probeIndex / volume.probeCounts.x) % volume.probeCounts.z;
	
	return ivec3(x, y, planeIndex);
}

// Computes the normalized texture UVs within the Probe Irradiance and Probe Distance texture arrays
// given the probe index and 2D normalized octant coordinates [-1, 1]. Used when sampling the texture arrays.
// When infinite scrolling is enabled, probeIndex is expected to be the scroll adjusted probe index.
// Obtain the adjusted index with DDGIGetScrollingProbeIndex().
vec3 DDGIGetProbeUV(int probeIndex, vec2 octantCoordinates, int numProbeInteriorTexels, DDGIVolume volume)
{
	// Get the probe's texel coordinates, assuming one texel per probe.
	ivec3 coords = DDGIGetProbeTexelCoords(probeIndex, volume);
	
	// Add the border texels to get the total texels per probe.
	float numProbeTexels = numProbeInteriorTexels + 2.0;
	float textureWidth = numProbeTexels * volume.probeCounts.x;
	float textureHeight = numProbeTexels * volume.probeCounts.z;
	
	// Move to the center of the probe and move to the octant texel before normalizing.
	vec2 uv = vec2(coords.x * numProbeTexels, coords.y * numProbeTexels) + (numProbeTexels * 0.5);
	uv += octantCoordinates.xy * ((float)numProbeInteriorTexels * 0.5);
	uv /= vec2(textureWidth, textureHeight);
	return vec3(uv, coords.z);
}

// Computes irradiance for the given world-position using the given volume, surface bias, sampling direction, and volume resources.
vec3 DDGIGetVolumeIrradiance(vec3 worldPosition, vec3 direction, DDGIVolume volume)
{
	// Surface bias, useless for now.
	const vec3 surfaceBias = vec3_splat(0.0);
	
	vec3 irradiance = vec3_splat(0.0);
	float accumulatedWeights = 0.0;
	
	// Bias the world space position.
	vec3 biasedWorldPosition = worldPosition + surfaceBias;
	
	// Get the 3D grid coordinates of the probe nearest the biased world position (i.e. the "base" probe).
	ivec3 baseProbeCoords = DDGIGetBaseProbeGridCoords(biasedWorldPosition, volume);
	
	// Get the world-space position of the base probe (ignore relocation).
	vec3 baseProbeWorldPosition = DDGIGetProbeWorldPosition(baseProbeCoords, volume);
	
	// Clamp the distance (in grid space) between the given point and the base probe's world position (on each axis) to [0, 1].
	vec3 gridSpaceDistance = worldPosition - baseProbeWorldPosition;
	vec3 alpha = clamp((gridSpaceDistance / volume.probeSpacing), vec3_splat(0.0), vec3_splat(1.0));
	
	// Iterate over the 8 closest probes and accumulate their contributions
	for(int probeIndex = 0; probeIndex < 8; probeIndex++)
	{
		float weight = 1.0;
		
		// Compute the offset to the adjacent probe in grid coordinates by
		// sourcing the offsets from the bits of the loop index: x = bit 0, y = bit 1, z = bit 2.
		ivec3 adjacentProbeOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1, 1, 1);
		
		// Get the 3D grid coordinates of the adjacent probe by adding the offset to 
		// the base probe and clamping to the grid boundaries.
		ivec3 adjacentProbeCoords = clamp(baseProbeCoords + adjacentProbeOffset, ivec3(0, 0, 0), volume.probeCounts - ivec3(1, 1, 1));
		
		// Get the adjacent probe's index, adjusting the adjacent probe index for scrolling offsets (if present).
		// int adjacentProbeIndex = DDGIGetScrollingProbeIndex(adjacentProbeCoords, volume);
		int adjacentProbeIndex = DDGIGetProbeIndex(adjacentProbeCoords, volume);
		
		// Early Out: don't allow inactive probes to contribute to irradiance.
		
		// Get the adjacent probe's world position.
		vec3 adjacentProbeWorldPosition = DDGIGetProbeWorldPosition(adjacentProbeCoords, volume);
		
		// Compute the distance and direction from the (biased and non-biased) shading point and the adjacent probe.
		vec3  worldPosToAdjProbe = normalize(adjacentProbeWorldPosition - worldPosition);
		vec3  biasedPosToAdjProbe = normalize(adjacentProbeWorldPosition - biasedWorldPosition);
		float biasedPosToAdjProbeDist = length(adjacentProbeWorldPosition - biasedWorldPosition);
		
		// Compute trilinear weights based on the distance to each adjacent probe
		// to smoothly transition between probes. adjacentProbeOffset is binary, so we're
		// using a 1-alpha when adjacentProbeOffset = 0 and alpha when adjacentProbeOffset = 1.
		vec3  trilinear = max(0.001, lerp(1.0 - alpha, alpha, adjacentProbeOffset));
		float trilinearWeight = (trilinear.x * trilinear.y * trilinear.z);
		weight *= trilinearWeight;
		
		// A naive soft backface weight would ignore a probe when
		// it is behind the surface. That's good for walls, but for
		// small details inside of a room, the normals on the details
		// might rule out all of the probes that have mutual visibility 
		// to the point. We instead use a "wrap shading" test. The small
		// offset at the end reduces the "going to zero" impact.
		float wrapShading = (dot(worldPosToAdjProbe, direction) + 1.0) * 0.5;
		weight *= (wrapShading * wrapShading) + 0.2;
		
		// Compute the octahedral coordinates of the adjacent probe.
		vec2 octantCoords = DDGIGetOctahedralCoordinates(-biasedPosToAdjProbe);
		
		// Get the texture array coordinates for the octant of the probe.
		vec3 probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, DISTANCE_TEXELS_COUNT, volume);
		
		// Sample the probe's distance texture to get the mean distance to nearby surfaces.
		vec2 filteredDistance = 2.0 * SampleDistance(probeTextureUV.xy).xy;
		
		// Find the variance of the mean distance.
		float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);
		
		// Occlusion test.
		float chebyshevWeight = 1.0;
		if(biasedPosToAdjProbeDist > filteredDistance.x)
		{
			// v must be greater than 0, which is guaranteed by the if condition above.
			float v = biasedPosToAdjProbeDist - filteredDistance.x;
			chebyshevWeight = variance / (variance + (v * v));
		
			// Increase the contrast in the weight.
			chebyshevWeight = max((chebyshevWeight * chebyshevWeight * chebyshevWeight), 0.0);
		}
		
		// Avoid visibility weights ever going all the way to zero because
		// when *no* probe has visibility we need a fallback value.
		weight *= max(0.05, chebyshevWeight);
		// Avoid a weight of zero.
		weight = max(0.000001, weight);
		
		// A small amount of light is visible due to logarithmic perception, so
		// crush tiny weights but keep the curve continuous.
		const float crushThreshold = 0.2;
		if (weight < crushThreshold)
		{
			weight *= (weight * weight) * (1.0 / (crushThreshold * crushThreshold));
		}
		
		// Get the octahedral coordinates for the sample direction.
		octantCoords = DDGIGetOctahedralCoordinates(direction);
		
		// Get the probe's texture coordinates.
		probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, IRRADIANCE_TEXELS_COUNT, volume);
		
		// Sample the probe's irradiance.
		vec3 probeIrradiance = SampleIrradiance(probeTextureUV.xy);
		
		// Accumulate the weighted irradiance.
		irradiance += (weight * probeIrradiance);
		accumulatedWeights += weight;
	}
	
	if(accumulatedWeights <= 0.0)
		return vec3_splat(0.0);
	
	// Normalize by the accumulated weights.
	irradiance *= (1.0 / accumulatedWeights);
	// Go back to linear irradiance.
	irradiance *= irradiance;
	// Multiply by the area of the integration domain (hemisphere) to complete the Monte Carlo Estimator equation.
	irradiance *= (2.0 * PI);
	
	return irradiance;
}

void main()
{
	DDGIVolume volume = GetVolumeData();
	gl_FragColor = vec4(DDGIGetVolumeIrradiance(v_worldPos, v_normal, volume), 1.0);
}
