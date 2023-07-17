// @brief Get environment diffuse irradiance by ddgi textures.
// 
// vec3 GetDDGIIrradiance(vec3 worldPos, vec3 normal);

#include "../UniformDefines/U_DDGI.sh"

#define DDGI_TEXTURE_FROM_O3DE

uniform vec4 u_volumeOrigin;
uniform vec4 u_volumeProbeSpacing;
uniform vec4 u_volumeProbeCounts;
uniform vec4 u_ambientMultiplier;

SAMPLER2D(s_texClassification, CLA_MAP_SLOT);
SAMPLER2D(s_texDistance, DIS_MAP_SLOT);
SAMPLER2D(s_texIrradiance, IRR_MAP_SLOT);
SAMPLER2D(s_texRelocation, REL_MAP_SLOT);

vec3 SampleDistance(vec2 uv) {
	return texture2D(s_texDistance, uv).xyz;
}

vec3 SampleIrradiance(vec2 uv) {
	return texture2D(s_texIrradiance, uv).xyz;
}

vec3 DDGILoadProbeDataOffset(ivec2 coord) {
	vec3 offset = texelFetch(s_texRelocation, coord, 0).xyz;
#if defined(DDGI_TEXTURE_FROM_O3DE)
	return vec3(offset.x, offset.z, offset.y);
#else
	return offset;
#endif
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

// ---------------------------------------- Math ---------------------------------------- //

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
		uv = (vec2_splat(1.0) - abs(uv.yx)) * SignNotZero(uv);
	}
	return uv;
}

// ---------------------------------------- Probe Indexing ---------------------------------------- //

// Computes the 3D grid-space coordinates of the "base" probe (i.e. floor of xyz) of the 8-probe
// cube that surrounds the given world space position. The other seven probes of the cube
// are offset by 0 or 1 in grid space along each axis.
ivec3 DDGIGetBaseProbeGridCoords(vec3 worldPosition, DDGIVolume volume) {
	// Get the vector from the volume origin to the surface point.
	vec3 position = worldPosition - volume.origin;
	// Shift from [-n/2, n/2] to [0, n] (grid space).
	position += (volume.probeSpacing * vec3(volume.probeCounts - ivec3(1, 1, 1)) * vec3_splat(0.5));
	// Quantize the position to grid space.
	ivec3 probeCoords = ivec3(position / volume.probeSpacing);
	// Clamp to [0, probeCounts - 1].
	// Snaps positions outside of grid to the grid edge.
	probeCoords = clamp(probeCoords, ivec3(0, 0, 0), (volume.probeCounts - ivec3(1, 1, 1)));
	
	return probeCoords;
}

// Computes the probe index from 3D grid coordinates.
int DDGIGetProbeIndex(ivec3 probeCoords, ivec3 probeCounts) {
	int planeIndex = probeCoords.y;
	int probesPerPlane = probeCounts.x * probeCounts.z;
	int probeIndexInPlane = probeCounts.x * probeCoords.z + probeCoords.x;
	
	// Return [0, probeCounts.x * probeCounts.y * probeCounts.z - 1]
	return planeIndex * probesPerPlane + probeIndexInPlane;
}

// Computes the normalized texture UVs within the Probe Irradiance and Probe Distance texture arrays
// given the probe index and 2D normalized octant coordinates [-1, 1]. Used when sampling the texture arrays.
vec2 DDGIGetProbeUV(int probeIndex, vec2 octantCoordinates, int numProbeInteriorTexels, ivec3 probeCounts) {
	vec2 coords = vec2(probeIndex / probeCounts.x, probeIndex % probeCounts.x);
	
	// Add the border texels to get the total texels per probe.
	float numProbeTexels = float(numProbeInteriorTexels) + 2.0;
	float textureWidth = numProbeTexels * float(probeCounts.y * probeCounts.z);
	float textureHeight = numProbeTexels * float(probeCounts.x);
	
	// Move to the center of the probe and move to the octant texel before normalizing.
	vec2 uv = vec2(coords.x * numProbeTexels, coords.y * numProbeTexels) + vec2_splat(numProbeTexels * 0.5);
	float halfProbeTexels = float(numProbeInteriorTexels) * 0.5;
	uv += clamp(octantCoordinates * vec2_splat(halfProbeTexels), vec2_splat(-halfProbeTexels), vec2_splat(halfProbeTexels));
	uv /= vec2(textureWidth, textureHeight);
	return uv;
}

ivec2 DDGIGetProbeOffsetCoord(int probeIndex, ivec3 probeCounts) {
	int x = probeIndex / probeCounts.x;
	int y = probeIndex % probeCounts.x;
	return ivec2(x, y);
}

ivec2 DDGIGetProbeOffsetCoord(ivec3 probeCoords, ivec3 probeCounts) {
	int x = probeCoords.y * probeCounts.z + probeCoords.z;
	int y = probeCoords.x;
	return ivec2(x, y);
}

// ---------------------------------------- Word Position ---------------------------------------- //

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
	// Get the world-space position of the base probe (ignore relocation).
	vec3 baseProbeWorldPosition = DDGIGetProbeWorldPosition(baseProbeCoords, volume);
	
	// ivec2 tmpCoords = DDGIGetProbeOffsetCoord(baseProbeCoords, volume.probeCounts);
	// return DDGILoadProbeDataOffset(tmpCoords);
	
	// Clamp the distance (in grid space) between the given point and the base probe's world position (on each axis) to [0, 1].
	vec3 alpha = clamp(((worldPosition - baseProbeWorldPosition) / volume.probeSpacing), vec3_splat(0.0), vec3_splat(1.0));
	
	vec3 irradiance = vec3_splat(0.0);
	float accumulatedWeights = 0.0;
	
	// Iterate over the 8 closest probes and accumulate their contributions
	for(int probeIndex = 0; probeIndex < 8; probeIndex++) {
		// Compute the offset to the adjacent probe in grid coordinates by
		// sourcing the offsets from the bits of the loop index: x = bit 0, y = bit 1, z = bit 2.
		// 0:(0,0,0) 1:(1,0,0) 2:(0,1,0) 3:(1,1,0) 4:(0,0,1) 5:(1,0,1) 6:(0,1,1) 7:(1,1,1)
		ivec3 adjacentProbeCoordsOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1, 1, 1);
		
		// Get the 3D grid coordinates of the adjacent probe by adding the offset to
		// the base probe and clamping to the grid boundaries.
		ivec3 adjacentProbeCoords = clamp(baseProbeCoords + adjacentProbeCoordsOffset, ivec3(0, 0, 0), volume.probeCounts - ivec3(1, 1, 1));
		// Get the adjacent probe's world position.
		vec3 adjacentProbeWorldPosition = DDGIGetProbeWorldPosition(adjacentProbeCoords, volume);
		
		// Find the texture coordinates of the probe.
		ivec2 adjacentProbeTextureCoords = DDGIGetProbeOffsetCoord(adjacentProbeCoords, volume.probeCounts);
		// Load the probe's world-space position offset and add it to the current world position.
		// We dont have Classification and Relocation texture for now.
		// adjacentProbeWorldPosition += (DDGILoadProbeDataOffset(adjacentProbeTextureCoords) * volume.probeSpacing);
		
		// Compute the distance and direction from the (biased and non-biased) shading point and the adjacent probe.
		vec3  worldPosToAdjProbe = normalize(adjacentProbeWorldPosition - worldPosition);
		vec3  biasedPosToAdjProbe = normalize(adjacentProbeWorldPosition - biasedWorldPosition);
		float biasedPosToAdjProbeDist = length(adjacentProbeWorldPosition - biasedWorldPosition);
		
		// 1. Distance
		// Compute trilinear weights based on the distance to each adjacent probe
		// to smoothly transition between probes. adjacentProbeCoordsOffset is binary, so we're
		// using a 1-alpha when adjacentProbeCoordsOffset = 0 and alpha when adjacentProbeCoordsOffset = 1.
		vec3  trilinear = max(mix(vec3_splat(1.0) - alpha, alpha, vec3(adjacentProbeCoordsOffset)), vec3_splat(0.0001));
		float trilinearWeight = trilinear.x * trilinear.y * trilinear.z;
		
		// 2. Normal
		// A naive soft backface weight would ignore a probe when
		// it is behind the surface. That's good for walls, but for
		// small details inside of a room, the normals on the details
		// might rule out all of the probes that have mutual visibility 
		// to the point. We instead use a "wrap shading" test. The small
		// offset at the end reduces the "going to zero" impact.
		float wrapShading = (dot(worldPosToAdjProbe, direction) + 1.0) * 0.5;
		float weight = wrapShading * wrapShading + 0.2;
		
		// 3. Chebyshev
		// Compute the octahedral coordinates of the adjacent probe.
		vec2 octantCoords = DDGIGetOctahedralCoordinates(-biasedPosToAdjProbe);
		// Get the probe index.
		int adjacentProbeIndex = DDGIGetProbeIndex(adjacentProbeCoords, volume.probeCounts);
		// Get the texture array coordinates for the octant of the probe.
		vec2 probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, DISTANCE_GRID_SIZE - 2, volume.probeCounts);
		// Sample the probe's distance texture to get the mean distance to nearby surfaces.
		vec2 filteredDistance = SampleDistance(probeTextureUV).xy;
		
		// Occlusion test.
		float chebyshevWeight = 1.0;
		if(biasedPosToAdjProbeDist > filteredDistance.x) {
			// Find the variance of the mean distance.
			float variance = max(abs(filteredDistance.x * filteredDistance.x - filteredDistance.y), 0.0001);
			
			// v must be greater than 0, which is guaranteed by the if condition above.
			float v = biasedPosToAdjProbeDist - filteredDistance.x;
			chebyshevWeight = variance / (variance + v * v);
			// Increase the contrast in the weight.
			chebyshevWeight = clamp(chebyshevWeight * chebyshevWeight * chebyshevWeight, 0.0, 1.0);
		}
		
		// Avoid visibility weights ever going all the way to zero because
		// when *no* probe has visibility we need a fallback value.
		weight *= max(chebyshevWeight, 0.0001);
		weight = max(weight, 0.0001);
		
		// A small amount of light is visible due to logarithmic perception, so
		// crush tiny weights but keep the curve continuous.
		const float crushThreshold = 0.2;
		if (weight < crushThreshold) {
			weight *= (weight * weight / (crushThreshold * crushThreshold));
		}
		
		weight *= trilinearWeight;
		
		// Get the octahedral coordinates for the sample direction.
		octantCoords = DDGIGetOctahedralCoordinates(direction);
		
		// Get the probe's texture coordinates.
		probeTextureUV = DDGIGetProbeUV(adjacentProbeIndex, octantCoords, IRRADIANCE_GRID_SIZE - 2, volume.probeCounts);
		
		// Sample the probe's irradiance.
		vec3 probeIrradiance = SampleIrradiance(probeTextureUV);
		
		// Accumulate the weighted irradiance.
		irradiance += (probeIrradiance * weight);
		accumulatedWeights += weight;
	}
	
	if(accumulatedWeights <= 0.0) {
		return vec3_splat(0.0);
	}
	
	// Normalize by the accumulated weights.
	irradiance /= vec3_splat(accumulatedWeights);
	
	// Go back to linear irradiance.
	// irradiance *= irradiance;
	
	// Multiply by the area of the integration domain (hemisphere) to complete the Monte Carlo Estimator equation.
	irradiance *= vec3_splat(2.0 * CD_PI);
	
	return irradiance;
}

vec3 GetDDGIIrradiance(vec3 worldPos, vec3 normal) {
	DDGIVolume volume = GetVolumeData();
	
	vec3 irradiance = DDGIGetVolumeIrradiance(worldPos, normal, volume);
	
	vec3 originToFrag = worldPos - volume.origin;
	vec3 volumeHalfLengths = vec3(volume.probeCounts) * volume.probeSpacing * vec3_splat(0.5);
	if(abs(dot(vec3(1.0, 0.0, 0.0), originToFrag)) > volumeHalfLengths.x
		|| abs(dot(vec3(0.0, 1.0, 0.0), originToFrag)) > volumeHalfLengths.y
		|| abs(dot(vec3(0.0, 0.0, 1.0), originToFrag)) > volumeHalfLengths.z)
	{
		return vec3_splat(0.0);
	}
	
	return irradiance * u_ambientMultiplier.x;
}
