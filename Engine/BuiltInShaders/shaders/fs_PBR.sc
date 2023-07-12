$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/Camera.sh"
#include "../common/Light.sh"

uniform vec4 u_emissiveColor;

#if defined(IBL)
SAMPLERCUBE(s_texCubeIrr, IBL_IRRADIANCE_SLOT);
SAMPLERCUBE(s_texCube, IBL_RADIANCE_SLOT);

vec3 SampleEnvIrradiance(vec3 normal, float mip) {
	vec3 cubeNormalDir = normalize(fixCubeLookup(normal, mip, 256.0));
	return toLinear(textureCube(s_texCubeIrr, cubeNormalDir).xyz);
}

vec3 SampleEnvRadiance(vec3 reflectDir, float mip) {
	vec3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
	return toLinear(textureCubeLod(s_texCube, cubeReflectDir, mip).xyz);
}
#endif

SAMPLER2D(s_texLUT, LUT_SLOT);

vec2 SampleIBLSpecularBRDFLUT(float NdotV, float roughness) {
	return texture2D(s_texLUT, vec2(NdotV, 1.0 - roughness)).xy;
}

vec3 GetDirectional(Material material, vec3 worldPos, vec3 viewDir) {
	vec3 diffuseBRDF = material.albedo * CD_INV_PI;
	return CalculateLights(material, worldPos, viewDir, diffuseBRDF);
}

vec3 GetEnvironment(Material material, vec3 vertexNormal, vec3 viewDir) {
	vec3 envIrradiance = vec3_splat(1.0);
	vec3 envRadiance = vec3_splat(1.0);
	vec3 reflectDir = normalize(reflect(-viewDir, material.normal));
	
#if defined(IBL)
	float mip = clamp(6.0 * material.roughness, 0.1, 6.0);
	
	// Environment Prefiltered Irradiance
	envIrradiance = SampleEnvIrradiance(material.normal, mip);
	// Environment Specular Radiance
	envRadiance = SampleEnvRadiance(reflectDir, mip);
#endif
	
	// Environment Specular BRDF
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	vec2 lut = SampleIBLSpecularBRDFLUT(NdotV, material.roughness);
	vec3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
	
	// Occlusion
	float specularOcclusion = lerp(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, vertexNormal));
	horizonOcclusion *= horizonOcclusion;
	float finalOcclusion = min(specularOcclusion, horizonOcclusion);
	
	vec3 Fre = FresnelSchlick(NdotV, material.F0);
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	
	// diffuse + specular
	return (KD * material.albedo * envIrradiance * material.occlusion) + (envSpecularBRDF * envRadiance * finalOcclusion);
}

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	
#if defined(ORM_MAP)
	// Directional Light
	vec3 dirColor = GetDirectional(material, v_worldPos, viewDir);
	
	// Environment Light
	vec3 envColor = GetEnvironment(material, v_normal, viewDir);
	
	// Emissive
	vec3 emiColor = material.emissive * u_emissiveColor.xyz;
	
	// Fragment Color
	gl_FragColor = vec4(dirColor + envColor + emiColor, 1.0);
#else
	gl_FragColor = vec4(material.albedo, 1.0);
#endif
	
	// Post-processing will be used in the last pass.
}
