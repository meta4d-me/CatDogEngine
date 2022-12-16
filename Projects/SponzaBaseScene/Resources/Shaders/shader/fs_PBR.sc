$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

#if defined(USE_LIGHT)
	#define POINT_LIGHT_LENGTH 32
	#define SPOT_LIGHT_LENGTH 48
	#define DIRECTIONAL_LIGHT_LENGTH 16
	#define SPHERE_LIGHT_LENGTH 48
	#define DISK_LIGHT_LENGTH 48
	#define RECTANGLE_LIGHT_LENGTH 64
	#define TUBE_LIGHT_LENGTH 64
#endif

uniform vec4 u_pointLightCount[1];
uniform vec4 u_pointLightStride[1];
uniform vec4 u_spotLightCount[1];
uniform vec4 u_spotLightStride[1];
uniform vec4 u_directionalLightCount[1];
uniform vec4 u_directionalLightStride[1];
uniform vec4 u_sphereLightCount[1];
uniform vec4 u_sphereLightStride[1];
uniform vec4 u_diskLightCount[1];
uniform vec4 u_diskLightStride[1];
uniform vec4 u_rectangleLightCount[1];
uniform vec4 u_rectangleLightStride[1];
uniform vec4 u_tubeLightCount[1];
uniform vec4 u_tubeLightStride[1];

SAMPLERCUBE(s_texCube, 0);
SAMPLERCUBE(s_texCubeIrr, 1);
SAMPLER2D(s_texBaseColor, 2);
SAMPLER2D(s_texNormal, 3);
SAMPLER2D(s_texORM, 4);
SAMPLER2D(s_texLUT, 5);

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
	material.albedo = vec3(0.99, 0.98, 0.95);
	material.normal = vec3(0.0, 1.0, 0.0);
	material.occlusion = 1.0;
	material.roughness = 0.9;
	material.metallic = 0.1;
	material.F0 = vec3(0.04, 0.04, 0.04);
	material.opacity = 1.0;
	material.emissive = vec3_splat(0.0);
	return material;
}

vec3 SampleAlbedoTexture(vec2 uv) {
	// We assume that albedo texture is already in linear space.
	return texture2D(s_texBaseColor, uv).xyz;
}

vec3 CalcuateNormal(vec3 worldPos) {
	vec3 dx = dFdx(worldPos);
	vec3 dy = dFdy(worldPos);
	return normalize(cross(dx, dy));
}

vec3 SampleNormalTexture(vec2 uv, mat3 TBN) {
	vec3 normalTexture = normalize(texture2D(s_texNormal, uv).xyz * 2.0 - 1.0);
	return normalize(mul(TBN, normalTexture));
}

vec3 SampleORMTexture(vec2 uv) {
	// We assume that ORM texture is already in linear space.
	vec3 orm = texture2D(s_texORM, uv).xyz;
	orm.y = clamp(orm.y, 0.04, 1.0); // roughness
	return orm;
}

vec3 CalcuateF0(Material material) {
	return mix(vec3_splat(0.04), material.albedo, material.metallic);
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

// Angle Attenuation
float GetAngleAtt(vec3 lightDir, vec3 lightForward, float lightAngleScale, float lightAngleOffeset) {
	// On CPU
	// float lightAngleScale = 1.0f / max(0.001f, cosInner - cosOuter);
	// float lightAngleOffeset = -cosOuter * angleScale;
	
	float cd = dot(lightDir, lightForward);
	float attenuation = saturate(cd * lightAngleScale + lightAngleOffeset);
	attenuation *= attenuation;
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
	float denom = (NdotH * NdotH * (a2 - 1.0) + 1.0);
	denom = PI * denom * denom;
	return a2 / denom;
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

float RectangleSolidAngle(vec3 worldPos , vec3 p0 , vec3 p1 ,vec3 p2 , vec3 p3) {
	vec3 v0 = p0 - worldPos;
	vec3 v1 = p1 - worldPos;
	vec3 v2 = p2 - worldPos;
	vec3 v3 = p3 - worldPos;
	vec3 n0 = normalize(cross(v0, v1));
	vec3 n1 = normalize(cross(v1, v2));
	vec3 n2 = normalize(cross(v2, v3));
	vec3 n3 = normalize(cross(v3, v0));
	float g0 = acos(dot(-n0, n1));
	float g1 = acos(dot(-n1, n2));
	float g2 = acos(dot(-n2, n3));
	float g3 = acos(dot(-n3, n0));
	return g0 + g1 + g2 + g3 - 2.0 * PI;
}

#include "lights/pointLight.sh"
#include "lights/spotLight.sh"
#include "lights/directionalLight.sh"
#include "lights/sphereLight.sh"
#include "lights/diskLight.sh"
#include "lights/rectangleLight.sh"
#include "lights/tubeLight.sh"

void main()
{
	Material material = CreateMaterial();
	material.albedo = SampleAlbedoTexture(v_texcoord0);
	material.normal = SampleNormalTexture(v_texcoord0, v_TBN);
	vec3 orm = SampleORMTexture(v_texcoord0);
	material.occlusion = orm.x;
	material.roughness = orm.y;
	material.metallic = orm.z;
	material.F0 = CalcuateF0(material);
	
	vec3 viewDir  = normalize(u_cameraPos - v_worldPos);
	vec3 diffuseBRDF = material.albedo * INV_PI;
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float mip = clamp(6.0 * material.roughness, 0.1, 6.0);
	
	// ------------------------------------ Directional Light ----------------------------------------
	
	vec3 dirColor = vec3_splat(0.0);
	
#if defined(USE_LIGHT)
	dirColor += CalculatePointLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateSpotLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateDirectionalLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateSphereLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateDiskLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateRectangleLights(material, v_worldPos, viewDir, diffuseBRDF);
	dirColor += CalculateTubeLights(material, v_worldPos, viewDir, diffuseBRDF);
#endif
	
	// ----------------------------------- Environment Light ----------------------------------------
	
	// Environment Prefiltered Irradiance
	vec3 cubeNormalDir = normalize(fixCubeLookup(material.normal, mip, 256.0));
	vec3 envIrradiance = toLinear(textureCube(s_texCubeIrr, cubeNormalDir).xyz);
	
	// Environment Specular BRDF
	vec2 lut = texture2D(s_texLUT, vec2(NdotV, 1.0 - material.roughness)).xy;
	vec3 envSpecularBRDF = (material.F0 * lut.x + lut.y);
	
	// Environment Specular Radiance
	vec3 reflectDir = normalize(reflect(-viewDir, material.normal));
	vec3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
	vec3 envRadiance = toLinear(textureCubeLod(s_texCube, cubeReflectDir, mip).xyz);
	
	// Occlusion
	float specularOcclusion = lerp(pow(material.occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, v_normal));
	horizonOcclusion *= horizonOcclusion;
	
	vec3 F = FresnelSchlick(NdotV, material.F0);
	vec3 KD = mix(1.0 - F, vec3_splat(0.0), material.metallic);
	
	vec3 envColor = KD * material.albedo * envIrradiance * material.occlusion + envSpecularBRDF * envRadiance * min(specularOcclusion, horizonOcclusion);
	
	// ------------------------------------ Fragment Color -----------------------------------------
	
	gl_FragColor = vec4(dirColor + envColor, 1.0);
}
