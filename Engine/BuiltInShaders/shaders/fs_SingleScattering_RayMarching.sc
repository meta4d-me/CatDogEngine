$input v_worldPos

#include "../common/common.sh"
#include "../common/Camera.sh"

uniform vec4 u_LightDir;

// Unit : km
#define _PlanetRadius 6371.0
#define _AtmosphereHeight 80.0
#define _DensityScaleHeight vec2(7.994, 1.2)
#define _PlanetCenter vec3(0.0, -_PlanetRadius, 0.0)

#define _ExtinctionR vec3(0.0058, 0.0135, 0.0331)
#define _ExtinctionM vec3(0.0200, 0.0200, 0.0200)

// normal in [0.75, 0.99], can not be -1 or 1
#define _MieG 0.98
#define _IncomingLight vec3(12.0, 12.0, 12.0)
#define _PI 3.1415926

float GetAltitude(vec3 position){
	return abs(length(position - _PlanetCenter) - _PlanetRadius);
}

vec2 RaySphereIntersection(vec3 rayOrigin, vec3 rayDir, vec3 sphereCenter, float sphereRadius) {
	rayOrigin -= sphereCenter;
	float a = dot(rayDir, rayDir);
	float b = 2.0 * dot(rayOrigin, rayDir);
	float c = dot(rayOrigin, rayOrigin) - (sphereRadius * sphereRadius);
	float d = b * b - 4.0 * a * c;
	if (d < 0.0) {
		return vec2(-1.0, -1.0);
	}
	else {
		d = sqrt(d);
		return vec2(-b - d, -b + d) / (2.0 * a);
	}
}

vec2 RayEarthIntersection(vec3 rayOrigin, vec3 rayDir) {
	return RaySphereIntersection(rayOrigin, rayDir, _PlanetCenter, _PlanetRadius);
}

vec2 RayAtmosphereIntersection(vec3 rayOrigin, vec3 rayDir) {
	return RaySphereIntersection(rayOrigin, rayDir, _PlanetCenter, _PlanetRadius + _AtmosphereHeight);
}

// Optical depth point to point.
vec2 DensitySampling(vec3 startPos, vec3 endPos) {
	vec2 opticalDepth = vec2_splat(0.0);
	
	const int stepCount = 16;
	vec3 step = (endPos - startPos) / stepCount;
	float stepSize = length(step);
	
	for(int s = 0; s <= stepCount; ++s) {
		vec3 position = startPos + step * s;
		float height = GetAltitude(position);
		vec2 localDensity = exp(-vec2_splat(height) / _DensityScaleHeight.xy);
		float weight = (s == 0 || s == stepCount) ? 0.5 : 1.0;
		opticalDepth += localDensity * stepSize * weight;
	}
	
	return opticalDepth;
}

// Optical depth point to atmosphere top boundary.
vec2 LightDensitySampleing(vec3 position, vec3 lightDir) {
	// Assume this ray not intersect with the Earth.
	vec2 atmIntersection = RayAtmosphereIntersection(position, lightDir);
	vec3 rayEnd = position + lightDir * atmIntersection.y;
	return DensitySampling(position, rayEnd);
}

void ComputeLocalInscattering(vec2 rho, vec2 densityCP, vec2 densityPA, out vec3 localInscatterR, out vec3 localInscatterM) {
	vec2 densityCPA = densityCP + densityPA;
	
	vec3 Tr = densityCPA.x * _ExtinctionR;
	vec3 Tm = densityCPA.y * _ExtinctionM;

	vec3 extinction = exp(-(Tr + Tm));
		
	localInscatterR = extinction * rho.x;
	localInscatterM = extinction * rho.y;
}

void ApplyPhaseFunction(inout vec3 scatterR, inout vec3 scatterM, float cosAngle) {
	float cosAngle2  = cosAngle * cosAngle;
	float g = _MieG;
	float g2 = g * g;
	
	float phase = (3.0 / (16.0 * _PI)) * (1.0 + cosAngle2);
	scatterR *= phase;

	phase = (1.0 / (4.0 * _PI)) * ((3.0 * (1.0 - g2)) / (2.0 * (2.0 + g2))) * ((1.0 + cosAngle2) / (pow(abs((1.0 + g2 - 2.0 * g * cosAngle)), 1.5)));
	scatterM *= phase;
}

vec3 IntegrateInscattering(vec3 rayStart, vec3 rayDir, float rayLength, vec3 lightDir) {
	// accumulate in loop
	vec3 scatterR  = vec3_splat(0.0);
	vec3 scatterM  = vec3_splat(0.0);
	vec2 densityPA = vec2_splat(0.0);
	vec2 densityCP = vec2_splat(0.0);
	
	// P - current integration point
	// A - camera position
	// C - top of the atmosphere
	const int sampleCount = 256;
	vec3 step = rayDir * (rayLength / sampleCount);
	float stepSize = length(step);
	for (int s = 0; s <= sampleCount; ++s) {
		vec3 samplePos = rayStart + step * s;
		
		// We just compute single Scattering in this shader,
		// so a sample point provide no contribution if it can not directly see the sun.
		vec2 crtEarthIntersection = RayEarthIntersection(samplePos, lightDir);
		bool vis = (crtEarthIntersection.x > 0.0 || crtEarthIntersection.y > 0.0) ? false : true;
		
		if(vis) {
			densityCP = LightDensitySampleing(samplePos, lightDir);
			
			// DensityPA is a simple accumulation, no need to do redundant integration.
			float height = GetAltitude(samplePos);
			vec2 localDensityPA = exp(-vec2_splat(height) / _DensityScaleHeight.xy);
			densityPA += localDensityPA * stepSize;
			
			// localDensityPA equals to rho(h)
			vec3 localInscatterR;
			vec3 localInscatterM;
			ComputeLocalInscattering(localDensityPA, densityCP, densityPA, localInscatterR, localInscatterM);
			
			float weight = (s == 0 || s == sampleCount) ? 0.5 : 1.0;
			scatterR += localInscatterR * stepSize * weight;
			scatterM += localInscatterM * stepSize * weight;
		}
	}
	ApplyPhaseFunction(scatterR, scatterM, dot(rayDir, lightDir));
	return (scatterR * _ExtinctionR + scatterM * _ExtinctionM) * _IncomingLight.xyz;
}

void main()
{
	vec3 rayStart = GetCamera().position;
	vec3 rayDir = normalize(v_worldPos.xyz);
	
	// If the camera is in space, move it to the atmosphere top boundry along rayDir.
	vec2 atmIntersection = RayAtmosphereIntersection(rayStart, rayDir);
	if(atmIntersection.x > 0.0) {
		rayStart +=  rayDir * atmIntersection.x;
		atmIntersection = RayAtmosphereIntersection(rayStart, rayDir);
	}
	float rayLength = atmIntersection.y;
	
	// If rayDir intersect whith the earth.
	vec2 earthIntersection = RayEarthIntersection(rayStart, rayDir);
	if (earthIntersection.x > 0.0 || earthIntersection.y > 0.0) {
		rayLength = earthIntersection.x;
	}
	
	vec3 lightDir = -normalize(u_LightDir.xyz);
	vec3 inscattering = IntegrateInscattering(rayStart, rayDir, rayLength, lightDir);
	
	gl_FragColor = vec4(inscattering, 1.0);
}
