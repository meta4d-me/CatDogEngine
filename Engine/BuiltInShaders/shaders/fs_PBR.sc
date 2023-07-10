$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/Camera.sh"
#include "../UniformDefines/U_PBR.sh"

uniform vec4 u_albedoColor;
uniform vec4 u_emissiveColor;
uniform vec4 u_albedoUVOffsetAndScale;

#if defined(ALBEDO_MAP)
SAMPLER2D(s_texBaseColor, ALBEDO_MAP_SLOT);

vec3 SampleAlbedoTexture(vec2 uv) {
	// We assume that albedo texture is already in linear space.
	return texture2D(s_texBaseColor, uv).xyz;
}
#endif

#if defined(NORMAL_MAP)
SAMPLER2D(s_texNormal, NORMAL_MAP_SLOT);

vec3 SampleNormalTexture(vec2 uv, mat3 TBN) {
	// We assume that normal texture is already in linear space.
	vec3 normalTexture = normalize(texture2D(s_texNormal, uv).xyz * 2.0 - 1.0);
	return normalize(mul(TBN, normalTexture));
}
#endif

#if defined(ORM_MAP)
// ORM means that the three attributes of
// Occlusion, Roughness and Metallic are located in the
// R, G and B channels of the ORM texture.
SAMPLER2D(s_texORM, ORM_MAP_SLOT);

vec3 SampleORMTexture(vec2 uv) {
	// We assume that ORM texture is already in linear space.
	vec3 orm = texture2D(s_texORM, uv).xyz;
	
	// Clamp rouphness.
	orm.y = clamp(orm.y, 0.04, 1.0);
	
	return orm;
}
#endif

#if defined(EMISSIVE_MAP)
SAMPLER2D(s_texEmissive, EMISSIVE_MAP_SLOT);

vec3 SampleEmissiveTexture(vec2 uv) {
	// We assume that emissive texture is already in linear space.
	return texture2D(s_texEmissive, uv).xyz;
}
#endif

#if defined(IBL)
SAMPLERCUBE(s_texCube, IBL_ALBEDO_SLOT);
SAMPLERCUBE(s_texCubeIrr, IBL_IRRADIANCE_SLOT);
#endif

SAMPLER2D(s_texLUT, LUT_SLOT);
vec2 SampleIBLSpecularBRDFLUT(float NdotV, float roughness) {
	return texture2D(s_texLUT, vec2(NdotV, 1.0 - roughness)).xy;
}

struct Material {
	vec3 albedo;
	vec3 normal;
	float occlusion;
	float roughness;
	float metallic;
	vec3 F0;
	float opacity;
	vec3 emissive;
};

Material CreateMaterial() {
	Material material;
	material.albedo = vec3(1.0, 1.0, 1.0);
	material.normal = vec3(0.0, 1.0, 0.0);
	material.occlusion = 1.0;
	material.roughness = 0.9;
	material.metallic = 0.1;
	material.F0 = vec3(0.04, 0.04, 0.04);
	material.opacity = 1.0;
	material.emissive = vec3_splat(0.0);
	return material;
}

vec3 CalcuateF0(vec3 albedo, float metallic) {
	return mix(vec3_splat(0.04), albedo, metallic);
} 

Material GetMaterial(vec2 uv, vec3 normal, mat3 TBN) {
	Material material = CreateMaterial();

	vec2 uvOffset = vec2(u_albedoUVOffsetAndScale.x, u_albedoUVOffsetAndScale.y);
	vec2 uvScale = vec2(u_albedoUVOffsetAndScale.z, u_albedoUVOffsetAndScale.w);
	vec2 albedoUV = uv * uvScale + uvOffset;
	
#if defined(ALBEDO_MAP)
	material.albedo = SampleAlbedoTexture(albedoUV);
#endif
	material.albedo *= u_albedoColor.xyz;

#if defined(NORMAL_MAP)
	// Same to unity standard PBR, let normal uv same with albedo uv.
	material.normal = SampleNormalTexture(albedoUV, TBN);
#else
	material.normal = normal;
#endif
	
#if defined(ORM_MAP)
	vec3 orm = SampleORMTexture(uv);
	material.occlusion = orm.x;
	material.roughness = orm.y;
	material.metallic = orm.z;
#endif

#if defined(EMISSIVE_MAP)
	material.emissive = SampleEmissiveTexture(uv);
#endif
	
	material.F0 = CalcuateF0(material.albedo, material.metallic);
	
	return material;
}

#include "../common/light.sh"

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
