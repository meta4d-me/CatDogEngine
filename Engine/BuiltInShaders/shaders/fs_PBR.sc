$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/Camera.sh"
#include "../common/Light.sh"

uniform vec4 u_emissiveColor;

#if defined(IBL)
SAMPLERCUBE(s_texCube, IBL_ALBEDO_SLOT);
SAMPLERCUBE(s_texCubeIrr, IBL_IRRADIANCE_SLOT);
#endif

SAMPLER2D(s_texLUT, LUT_SLOT);
vec2 SampleIBLSpecularBRDFLUT(float NdotV, float roughness) {
	return texture2D(s_texLUT, vec2(NdotV, 1.0 - roughness)).xy;
}

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	
	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	vec3 reflectDir = normalize(reflect(-viewDir, material.normal));
	
	// ----------------------------------- Environment Light ----------------------------------------
	
	vec3 envIrradiance = vec3_splat(1.0);
	vec3 envRadiance = vec3_splat(1.0);
	
#if defined(IBL)
	float mip = clamp(6.0 * material.roughness, 0.1, 6.0);
	
	// Environment Prefiltered Irradiance
	vec3 cubeNormalDir = normalize(fixCubeLookup(material.normal, mip, 256.0));
	envIrradiance = toLinear(textureCube(s_texCubeIrr, cubeNormalDir).xyz);
	
	// Environment Specular Radiance
	vec3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
	envRadiance = toLinear(textureCubeLod(s_texCube, cubeReflectDir, mip).xyz);
#endif
	
	// Environment Specular BRDF
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	vec2 lut = SampleIBLSpecularBRDFLUT(NdotV, material.roughness);
	vec3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
	
	// Occlusion
	float specularOcclusion = lerp(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, v_normal));
	horizonOcclusion *= horizonOcclusion;
	
	vec3 F = FresnelSchlick(NdotV, material.F0);
	vec3 KD = mix(1.0 - F, vec3_splat(0.0), material.metallic);
	
#if defined(ORM_MAP)
	vec3 envColor = KD * material.albedo * envIrradiance * material.occlusion + envSpecularBRDF * envRadiance * min(specularOcclusion, horizonOcclusion);
#else
	// ORM is required or we will just show albedo color.
	vec3 envColor = material.albedo;
#endif
	
	// ------------------------------------ Directional Light ----------------------------------------
	
	vec3 diffuseBRDF = material.albedo * CD_INV_PI;
	vec3 dirColor = CalculateLights(material, v_worldPos, viewDir, diffuseBRDF);
	
	// ------------------------------------ Fragment Color -----------------------------------------
	gl_FragColor = vec4(dirColor + envColor + material.emissive * u_emissiveColor, 1.0);
}

