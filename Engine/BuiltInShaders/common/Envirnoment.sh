//------------------------------------------------------------------------------------//
// @brief Calculates the contribution of Image Based Lighting if defined macro IBL.   //
//                                                                                    //
// vec3 GetIBL(Material material, vec3 vertexNormal, vec3 viewDir);                   //
//------------------------------------------------------------------------------------//
// @brief Calculates the contribution of Atmospheric Scattering if defined macro ATM. //
//                                                                                    //
// vec3 GetATM(Material material, vec3 vertexNormal, vec3 viewDir);                   //
//------------------------------------------------------------------------------------//

#if defined(ATM)

#include "../atm/atm_functions.sh"

uniform vec4 u_LightDir;
uniform vec4 u_HeightOffset;

#endif

#if defined(IBL)

#include "../UniformDefines/U_IBL.sh"

SAMPLERCUBE(s_texCubeIrr, IBL_IRRADIANCE_SLOT);
SAMPLERCUBE(s_texCubeRad, IBL_RADIANCE_SLOT);
SAMPLER2D(s_texLUT, BRDF_LUT_SLOT);

vec3 SampleEnvIrradiance(vec3 normal, float mip) {
	// We use the HDR texture which in linear space.
	vec3 cubeNormalDir = normalize(fixCubeLookup(normal, mip, 256.0));
	return textureCube(s_texCubeIrr, cubeNormalDir).xyz;
}

vec3 SampleEnvRadiance(vec3 reflectDir, float mip) {
	// We use the HDR texture which in linear space.
	vec3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
	return textureCubeLod(s_texCubeRad, cubeReflectDir, mip).xyz;
}

vec2 SampleIBLSpecularBRDFLUT(float NdotV, float roughness) {
	return texture2D(s_texLUT, vec2(NdotV, 1.0 - roughness)).xy;
}
#endif

vec3 GetIBL(Material material, vec3 vertexNormal, vec3 viewDir) {
	vec3 envColor = vec3_splat(0.0);
	
#if defined(IBL)
	vec3 reflectDir = normalize(reflect(-viewDir, material.normal));
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	
	// Occlusion
	float specularOcclusion = mix(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, vertexNormal));
	horizonOcclusion *= horizonOcclusion;
	float finalSpecularOcclusion = min(specularOcclusion, horizonOcclusion);
	
	float mip = clamp(6.0 * material.roughness, 0.1, 6.0);
	
	// Environment Prefiltered Irradiance
	vec3 envIrradiance = SampleEnvIrradiance(material.normal, 0.0);
	// Environment Specular Radiance
	vec3 envRadiance = SampleEnvRadiance(reflectDir, mip);
	
	// Environment Specular BRDF
	vec2 lut = SampleIBLSpecularBRDFLUT(NdotV, material.roughness);
	vec3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
	
	vec3 Fre = FresnelSchlick(NdotV, material.F0);
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	
	// Diffuse
	envColor += (KD * material.albedo * envIrradiance * material.occlusion);
	
	// Specular
	envColor += (envSpecularBRDF * envRadiance * finalSpecularOcclusion);
#endif
	
	return envColor;
}

vec3 GetATM(Material material, vec3 worldPos) {
	vec3 envColor = vec3_splat(0.0);
	
#if defined(ATM)
	vec3 sunDir = normalize(u_LightDir.xyz);
	
	// Tramsform unit from cm to km.
	vec3 cameraPos = GetCamera().position / vec3_splat(100.0 * 100.0);
	vec3 worldPosOnEarth = worldPos / vec3_splat(100.0 * 100.0);
	// The coordinate system's origin of atmospheric scattering is the planet center.
	cameraPos += vec3(0.0, ATMOSPHERE.bottom_radius + u_HeightOffset.x, 0.0);
	worldPosOnEarth += vec3(0.0, ATMOSPHERE.bottom_radius, 0.0);
	
	// Irradiance from Sun and Sky
	vec3 irradiance = vec3_splat(0.0);
	vec3 sunIrradiance = GetSunAndSkyIrradiance(ATMOSPHERE, worldPosOnEarth, material.normal, sunDir, irradiance);
	irradiance += sunIrradiance;
	
	vec3 radiance = material.albedo / vec3_splat(PI) * irradiance;
	
	// Aerial perspective
	vec3 transmittance;
	// TODO : Need a shadow volume algorithm to get shadow_length parameter.
	vec3 skyRadianceToPoint = GetSkyRadianceToPoint(ATMOSPHERE, cameraPos, worldPosOnEarth, 0.1, sunDir, transmittance);
	radiance = radiance * transmittance + skyRadianceToPoint;
	
	envColor = radiance;
#endif
	
	return envColor;
}
