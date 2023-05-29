$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "fs_PBR_definitions.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

uniform vec4 u_albedoUVOffsetAndScale;
uniform vec4 u_cameraPos[1];
#define cameraPos u_cameraPos[0].xyz

uniform vec4 u_lightCount[1];
uniform vec4 u_lightStride[1];

#if defined(ALBEDO)
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

#if defined(USE_ORM)
SAMPLER2D(s_texORM, ORM_MAP_SLOT);

vec3 SampleORMTexture(vec2 uv) {
	// We assume that ORM texture is already in linear space.
	vec3 orm = texture2D(s_texORM, uv).xyz;
	orm.y = clamp(orm.y, 0.04, 1.0); // roughness
	
	return orm;
}
#endif

#if defined(EMISSIVE)
SAMPLER2D(s_texEmissive, EMISSIVE_MAP_SLOT);

vec3 SampleEmissiveTexture(vec2 uv) {
	// We assume that emissive texture is already in linear space.
	return texture2D(s_texEmissive, uv).xyz;
}
#endif

SAMPLER2D(s_texLUT, LUT_SLOT);

#if defined(IBL)
SAMPLERCUBE(s_texCube, IBL_ALBEDO_SLOT);
SAMPLERCUBE(s_texCubeIrr, IBL_IRRADIANCE_SLOT);
#endif

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

vec3 CalcuateNormal(vec3 worldPos) {
	vec3 dx = dFdx(worldPos);
	vec3 dy = dFdy(worldPos);
	return normalize(cross(dx, dy));
}

vec3 CalcuateF0(vec3 albedo, float metallic) {
	return mix(vec3_splat(0.04), albedo, metallic);
} 

Material GetMaterial(vec2 uv, vec3 normal, mat3 TBN) {
	Material material = CreateMaterial();

	vec2 uvOffset = vec2(u_albedoUVOffsetAndScale.x, u_albedoUVOffsetAndScale.y);
	vec2 uvScale = vec2(u_albedoUVOffsetAndScale.z, u_albedoUVOffsetAndScale.w);
	vec2 albedoUV = uv * uvScale + uvOffset;
#if defined(ALBEDO)
	material.albedo = SampleAlbedoTexture(albedoUV);
#endif

#if defined(NORMAL_MAP)
	// Same to unity standard PBR, let normal uv same with albedo uv.
	material.normal = SampleNormalTexture(albedoUV, TBN);
#else
	material.normal = normalize(normal);
#endif
	
#if defined(USE_ORM)
	vec3 orm = SampleORMTexture(uv);
	material.occlusion = orm.x;
	material.roughness = orm.y;
	material.metallic = orm.z;
#endif

#if defined(EMISSIVE)
	material.emissive = SampleEmissiveTexture(uv);
#endif
	
	material.F0 = CalcuateF0(material.albedo, material.metallic);
	
	return material;
}

// Distance Attenuation
float SmoothDistanceAtt(float squaredDistance, float invSqrAttRadius) {
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0 - factor * factor);
	return smoothFactor * smoothFactor;
}
float GetDistanceAtt(float sqrDist, float invSqrAttRadius) {
	float attenuation = 1.0 / (max(sqrDist , 0.0001));
	attenuation *= SmoothDistanceAtt(sqrDist, invSqrAttRadius);
	return attenuation;
}

// Fresnel
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Distribution
float DistributionGGX(float NdotH, float rough) {
	float a  = rough * rough;
	float a2 = a * a;
	float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
	return a2 / PI * denom * denom;
}

// Geometry
float Visibility(float NdotV, float NdotL, float rough) {
	// Specular BRDF = (F * D * G) / (4 * NdotV * NdotL)
	// = (F * D * (NdotV / (NdotV * (1 - K) + K)) * (NdotL / (NdotL * (1 - K) + K))) / (4 * NdotV * NdotL)
	// = (F * D * (1 / (NdotV * (1 - K) + K)) * (1 / (NdotL * (1 - K) + K))) / 4
	// = F * D * Vis
	// Vis = 1 / (NdotV * (1 - K) + K) / (NdotL * (1 - K) + K) / 4
	
	float f = rough + 1.0;
	float k = f * f * 0.125;
	float ggxV  = 1.0 / (NdotV * (1.0 - k) + k);
	float ggxL  = 1.0 / (NdotL * (1.0 - k) + k);
	return ggxV * ggxL * 0.25;
}

#include "light.sh"

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	
	vec3 viewDir  = normalize(cameraPos - v_worldPos);
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	
	// ------------------------------------ Directional Light ----------------------------------------
	
	vec3 diffuseBRDF = material.albedo * INV_PI;
	vec3 dirColor = CalculateLights(material, v_worldPos, viewDir, diffuseBRDF);
	
	// ----------------------------------- Environment Light ----------------------------------------

	// Environment Specular BRDF
	vec2 lut = texture2D(s_texLUT, vec2(NdotV, 1.0 - material.roughness)).xy;
	vec3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
	vec3 reflectDir = normalize(reflect(-viewDir, material.normal));

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

	// Occlusion
	float specularOcclusion = lerp(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, v_normal));
	horizonOcclusion *= horizonOcclusion;
	
	vec3 F = FresnelSchlick(NdotV, material.F0);
	vec3 KD = mix(1.0 - F, vec3_splat(0.0), material.metallic);
	
#if defined(USE_ORM)
	vec3 envColor = KD * material.albedo * envIrradiance * material.occlusion + envSpecularBRDF * envRadiance * min(specularOcclusion, horizonOcclusion);
#else
	// TODO : ORM is required or we will just show albedo color.
	vec3 envColor = material.albedo;
#endif
	
	// ------------------------------------ Fragment Color -----------------------------------------
	gl_FragColor = vec4(dirColor + envColor + material.emissive, 1.0);
}
