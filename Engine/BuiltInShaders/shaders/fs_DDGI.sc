$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define INV_PI 0.3183098862

#define DISTANCE_TEXELS 16
#define IRRADIANCE_TEXELS 8

uniform vec4 u_volumeOrigin;
uniform vec4 u_volumeProbeSpacing;
uniform vec4 u_volumeProbeCounts;

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texClassification, 1);
SAMPLER2D(s_texDistance, 2);
SAMPLER2D(s_texIrradiance, 3);
SAMPLER2D(s_texRelocation, 4);

vec3 SampleAlbedo(vec2 uv) {
	return texture2D(s_texBaseColor, uv).xyz;
}

vec3 SampleIrradiance(vec2 uv) {
	return texture2D(s_texIrradiance, uv).xyz;
}

vec3 SampleDistance(vec2 uv) {
	return texture2D(s_texDistance, uv).xyz;
}

vec3 DDGILoadProbeDataOffset(ivec2 coord, vec3 probeSpacing) {
	return texelFetch(s_texRelocation, coord, 0).xyz * probeSpacing;
}

struct DDGIVolume
{
	vec3 origin;
	vec3 probeSpacing;
	ivec3 probeCounts;
};

DDGIVolume GetVolumeData() {
	DDGIVolume volume;
	volume.origin = u_volumeOrigin.xyz;
	volume.probeSpacing = u_volumeProbeSpacing.xyz;
	volume.probeCounts = ivec3(u_volumeProbeCounts.xyz);
	return volume;
}

// ----- Math ----- //

// Returns either -1 or 1 based on the sign of the input value.
float SignNotZero(float v) {
	return (v >= 0.0) ? 1.0 : -1.0;
}
vec2 SignNotZero(vec2 v) {
	return vec2(SignNotZero(v.x), SignNotZero(v.y));
}

// Computes the normalized octahedral direction that corresponds to the given normalized coordinates on the [-1, 1] square.
vec3 DDGIGetOctahedralDirection(vec2 coords) {
	vec3 direction = vec3(coords.x, coords.y, 1.0 - abs(coords.x) - abs(coords.y));
	if (direction.z < 0.0) {
		direction.xy = (vec2_splat(1.0) - abs(direction.yx)) * SignNotZero(direction.xy);
	}
	return normalize(direction);
}

// Computes the octant coordinates in the normalized [-1, 1] square, for the given a unit direction vector.
vec2 DDGIGetOctahedralCoordinates(vec3 direction) {
	float l1norm = abs(direction.x) + abs(direction.y) + abs(direction.z);
	vec2 uv = direction.xy / vec2_splat(l1norm);
	if (direction.z < 0.0) {
		uv = (vec2_splat(1.0) - abs(uv.yx)) * SignNotZero(uv.xy);
	}
	return uv;
}

// ----- Probe Indexing ----- //

// Computes the 3D grid-space coordinates of the "base" probe (i.e. floor of xyz) of the 8-probe
// cube that surrounds the given world space position. The other seven probes of the cube
// are offset by 0 or 1 in grid space along each axis.
ivec3 DDGIGetBaseProbeGridCoords(vec3 worldPosition, DDGIVolume volume) {
	// Get the vector from the volume origin to the surface point.
	vec3 position = worldPosition - volume.origin;
	// Shift from [-n/2, n/2] to [0, n] (grid space).
	position += (volume.probeSpacing * (volume.probeCounts - 1)) * 0.5;
	// Quantize the position to grid space.
	ivec3 probeCoords = ivec3(position / volume.probeSpacing);
	// Clamp to [0, probeCounts - 1].
	// Snaps positions outside of grid to the grid edge.
	probeCoords = clamp(probeCoords, ivec3(0, 0, 0), (volume.probeCounts - ivec3(1, 1, 1)));
	
	return probeCoords;
}

// Computes the probe index from 3D grid coordinates.
// The opposite of DDGIGetProbeCoords(probeIndex,...).
int DDGIGetProbeIndex(ivec3 probeCoords, ivec3 probeCounts) {
	int planeIndex = probeCoords.y;
	int probesPerPlane = probeCounts.x * probeCounts.z;
	int probeIndexInPlane = probeCounts.x * probeCoords.z + probeCoords.x;
	
	// Return [0, probeCounts.x * probeCounts.y * probeCounts.z - 1]
	return planeIndex * probesPerPlane + probeIndexInPlane;
}

// Computes the Texture2DArray coordinates of the probe at the given probe index.
ivec3 DDGIGetProbeTexelCoords(int probeIndex, ivec3 probeCounts) {
	// Find the probe's plane index.
	int probesPerPlane = probeCounts.x * probeCounts.z;
	int planeIndex = int(probeIndex / probesPerPlane);
	
	int x = probeIndex % probeCounts.x;
	int y = (probeIndex / probeCounts.x) % probeCounts.z;
	
	return ivec3(x, y, planeIndex);
}

// Computes the normalized texture UVs within the Probe Irradiance and Probe Distance texture arrays
// given the probe index and 2D normalized octant coordinates [-1, 1]. Used when sampling the texture arrays.
vec2 DDGIGetProbeUV(int probeIndex, vec2 octantCoordinates, int numProbeInteriorTexels, ivec3 probeCounts) {
	// 30, (4, 2, 5)
	vec2 coords = vec2(probeIndex / probeCounts.x, probeIndex % probeCounts.x); // (7, 2)
	
	// Add the border texels to get the total texels per probe.
	float numProbeTexels = float(numProbeInteriorTexels) + 2.0; // 8
	float textureWidth = numProbeTexels * float(probeCounts.y * probeCounts.z); // 80
	float textureHeight = numProbeTexels * float(probeCounts.x); // 32
	
	// Move to the center of the probe and move to the octant texel before normalizing.
	vec2 uv = vec2(coords.x * numProbeTexels, coords.y * numProbeTexels) + vec2_splat(numProbeTexels * 0.5); // (56, 16) + (4, 4)
	uv += (octantCoordinates * vec2_splat(float(numProbeInteriorTexels) * 0.5));
	uv /= vec2(textureWidth, textureHeight);
	return uv;
}

ivec2 DDGIGetProbeCoord(int probeIndex, ivec3 probeCounts) {
	int x = probeIndex / probeCounts.x;
	int y = probeIndex % probeCounts.x;
	return ivec2(x, y);
}

// ----- Word Position ----- //

// Computes the world-space position of a probe from the probe's 3D grid-space coordinates.
// Probe relocation is not considered.
vec3 DDGIGetProbeWorldPosition(ivec3 probeCoords, DDGIVolume volume) {
	// Multiply the grid coordinates by the probe spacing.
	vec3 probeGridWorldPosition = probeCoords * volume.probeSpacing;
	// Shift the grid of probes by half of each axis extent to center the volume about its origin.
	vec3 probeGridShift = volume.probeSpacing * vec3(volume.probeCounts - ivec3(1, 1, 1)) * vec3_splat(0.5);
	// Center the probe grid about the origin.
	vec3 probeWorldPosition = probeGridWorldPosition - probeGridShift;
	// Translate the grid to the volume's center.
	probeWorldPosition += volume.origin;
	
	// return probeWorldPosition;
	
	// Get the probe index.
	int probeIndex = DDGIGetProbeIndex(probeCoords, volume.probeCounts);
	// Find the texture coordinates of the probe.
	ivec2 coords = DDGIGetProbeCoord(probeIndex, volume.probeCounts);
	// Load the probe's world-space position offset and add it to the current world position.
	vec3 offset = DDGILoadProbeDataOffset(coords, volume.probeSpacing);
	offset = vec3(offset.x, offset.z, offset.y);
	probeWorldPosition += DDGILoadProbeDataOffset(coords, volume.probeSpacing);
	
	return probeWorldPosition;
}

// Computes irradiance for the given world-position using the given volume, surface bias, sampling direction, and volume resources.
vec3 DDGIGetVolumeIrradiance(vec3 worldPosition, vec3 direction, DDGIVolume volume) {
	// Surface bias, useless for now.
	const vec3 surfaceBias = vec3_splat(0.0);
	
	// Bias the world space position.
	vec3 biasedWorldPosition = worldPosition + surfaceBias;
	
	// Get the 3D grid coordinates of the probe nearest the biased world position (i.e. the "base" probe).
	ivec3 baseProbeCoords = DDGIGetBaseProbeGridCoords(biasedWorldPosition, volume);
	// return vec3(baseProbeCoords) / vec3(volume.probeCounts - ivec3(1, 1, 1));
	
	// Get the world-space position of the base probe (ignore relocation).
	vec3 baseProbeWorldPosition = DDGIGetProbeWorldPosition(baseProbeCoords, volume);
	
	// Clamp the distance (in grid space) between the given point and the base probe's world position (on each axis) to [0, 1].
	vec3 gridSpaceDistance = worldPosition - baseProbeWorldPosition;
	vec3 alpha = clamp((gridSpaceDistance / volume.probeSpacing), vec3_splat(0.0), vec3_splat(1.0));
	
	vec3 irradiance = vec3_splat(0.0);
	float accumulatedWeights = 0.0;
	
	// Iterate over the 8 closest probes and accumulate their contributions
	for(int probeIndex = 0; probeIndex < 8; probeIndex++) {
		float weight = 1.0;
		
		// Compute the offset to the adjacent probe in grid coordinates by
		// sourcing the offsets from the bits of the loop index: x = bit 0, y = bit 1, z = bit 2.
		ivec3 adjacentProbeOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1, 1, 1);
		// 0:(0,0,0) 1:(1,0,0) 2:(0,1,0) 3:(1,1,0) 4:(0,0,1) 5:(1,0,1) 6:(0,1,1) 7:(1,1,1)
		
		// Get the 3D grid coordinates of the adjacent probe by adding the offset to 
		// the base probe and clamping to the grid boundaries.
		ivec3 adjacentProbeCoords = clamp(baseProbeCoords + adjacentProbeOffset, ivec3(0, 0, 0), volume.probeCounts - ivec3(1, 1, 1));
		
		// Get the adjacent probe's world position.
		vec3 adjacentProbeWorldPosition = DDGIGetProbeWorldPosition(adjacentProbeCoords, volume);
		
		// Compute the distance and direction from the (biased and non-biased) shading point and the adjacent probe.
		vec3  worldPosToAdjProbe = normalize(adjacentProbeWorldPosition - worldPosition);
		vec3  biasedPosToAdjProbe = normalize(adjacentProbeWorldPosition - biasedWorldPosition);
		float biasedPosToAdjProbeDist = length(adjacentProbeWorldPosition - biasedWorldPosition);
		
		// 1. Distance
		// Compute trilinear weights based on the distance to each adjacent probe
		// to smoothly transition between probes. adjacentProbeOffset is binary, so we're
		// using a 1-alpha when adjacentProbeOffset = 0 and alpha when adjacentProbeOffset = 1.
		vec3  trilinear = max(lerp(vec3_splat(1.0) - alpha, alpha, adjacentProbeOffset), 0.001);
		float trilinearWeight = trilinear.x * trilinear.y * trilinear.z;
		
		// 2. Normal
		// A naive soft backface weight would ignore a probe when
		// it is behind the surface. That's good for walls, but for
		// small details inside of a room, the normals on the details
		// might rule out all of the probes that have mutual visibility 
		// to the point. We instead use a "wrap shading" test. The small
		// offset at the end reduces the "going to zero" impact.
		float wrapShading = (dot(worldPosToAdjProbe, direction) + 1.0) * 0.5;
		weight *= (wrapShading * wrapShading + 0.2);
		
		// 3. Chebyshev
		// Get the adjacent probe's index.
		int adjacentProbeIndex = DDGIGetProbeIndex(adjacentProbeCoords, volume.probeCounts);
		// Compute the octahedral coordinates of the adjacent probe.
		vec2 octantCoords = DDGIGetOctahedralCoordinates(-biasedPosToAdjProbe);
		// Get the texture array coordinates for the octant of the probe.
		vec2 probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, DISTANCE_TEXELS - 2, volume.probeCounts);
		// Sample the probe's distance texture to get the mean distance to nearby surfaces.
		vec2 filteredDistance = 2.0 * SampleDistance(probeTextureUV).xy;
		
		// Find the variance of the mean distance.
		float variance = abs(filteredDistance.x * filteredDistance.x - filteredDistance.y);
		
		// Occlusion test.
		float chebyshevWeight = 1.0;
		if(biasedPosToAdjProbeDist > filteredDistance.x) {
			// v must be greater than 0, which is guaranteed by the if condition above.
			float v = biasedPosToAdjProbeDist - filteredDistance.x;
			chebyshevWeight = variance / (variance + v * v);
		
			// Increase the contrast in the weight.
			chebyshevWeight = max(chebyshevWeight * chebyshevWeight * chebyshevWeight, 0.0);
		}
		
		// Avoid visibility weights ever going all the way to zero because
		// when *no* probe has visibility we need a fallback value.
		weight *= max(chebyshevWeight, 0.05);
		// Avoid a weight of zero.
		weight = max(weight, 0.000001);
		
		// A small amount of light is visible due to logarithmic perception, so
		// crush tiny weights but keep the curve continuous.
		const float crushThreshold = 0.2;
		if (weight < crushThreshold) {
			weight *= (weight * weight * (1.0 / (crushThreshold * crushThreshold)));
		}
		
		weight *= trilinearWeight;
		
		// Get the octahedral coordinates for the sample direction.
		octantCoords = DDGIGetOctahedralCoordinates(direction);
		
		// Get the probe's texture coordinates.
		probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, IRRADIANCE_TEXELS - 2, volume.probeCounts);
		
		// Sample the probe's irradiance.
		vec3 probeIrradiance = SampleIrradiance(probeTextureUV);
		
		// Accumulate the weighted irradiance.
		irradiance += (weight * probeIrradiance);
		accumulatedWeights += weight;
	}
	
	if(accumulatedWeights <= 0.0) {
		return vec3_splat(0.0);
	}
	
	// Normalize by the accumulated weights.
	irradiance /= accumulatedWeights;
	
	// Go back to linear irradiance.
	//irradiance *= irradiance;
	
	// Multiply by the area of the integration domain (hemisphere) to complete the Monte Carlo Estimator equation.
	irradiance *= (2.0 * PI);
	
	return irradiance;
}

void main()
{
	DDGIVolume volume = GetVolumeData();
	
	// TODO : Its a temporary solution.
	vec3 fixedNormal = vec3(v_normal.x, v_normal.z, -v_normal.y);
	
	vec3 irradiance = DDGIGetVolumeIrradiance(v_worldPos, fixedNormal, volume);
	
	vec3 albedo = vec3(1.0, 1.0, 1.0);
	// albedo = SampleAlbedo(v_texcoord0);
	gl_FragColor = vec4(albedo * vec3_splat(INV_PI) * irradiance * vec3_splat(4.0), 1.0);
}
