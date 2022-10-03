$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926
#define INV_PI 0.3183091

SAMPLERCUBE(s_texCube, 0);
SAMPLERCUBE(s_texCubeIrr, 1);
SAMPLER2D(s_texBaseColor, 2);
SAMPLER2D(s_texNormal, 3);
SAMPLER2D(s_texORM, 4);
SAMPLER2D(s_texLUT, 5);

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

#if defined(POINT_LIGHT_LENGTH)
vec3 CalcPointLight(int pointer, vec3 worldPos, vec3 viewDir, vec3 normalDir, float roughness, float metallic, vec3 F0, vec3 dirLambert) {
	// struct {
	//	struct { float m_lightColor[3], m_intensity; };
	// 	struct { float m_lightPosition[3], m_radius; };
	// };
	 
	vec3  lightColor = u_pointLightParams[pointer + 0].xyz;
	float phi        = u_pointLightParams[pointer + 0].w;
	vec3  lightPos   = u_pointLightParams[pointer + 1].xyz;
	float radius     = u_pointLightParams[pointer + 1].w;
	
	vec3 lightDir = normalize(lightPos - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(normalDir, viewDir), 0.0);
	float NdotL = max(dot(normalDir, lightDir), 0.0);
	float NdotH = max(dot(normalDir, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(lightPos - worldPos);
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (radius * radius));
	vec3 radiance = lightColor * phi * 0.25 * INV_PI * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, F0);
	float NDF = DistributionGGX(NdotH, roughness);
	float Vis = Visibility(NdotV, NdotL, roughness);
	vec3 dirCookTorrance = Fre * NDF * Vis * u_doDirSpecular;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), metallic);
	
	return (KD * dirLambert + dirCookTorrance) * radiance * NdotL;
}
#endif

#if defined(SPOT_LIGHT_LENGTH)
vec3 CalcSpotLight(int pointer, vec3 worldPos, vec3 viewDir, vec3 normalDir, float roughness, float metallic, vec3 F0, vec3 dirLambert) {
	// struct {
	// 	struct { float m_lightColor[3], m_intensity; };
	// 	struct { float m_lightPosition[3], m_unused0; };
	// 	struct { float m_lightDirection[3], m_unused1; };
	// 	struct { float m_radius, m_innerCutOff, m_outerCutOff, m_unused2; };
	// };
	 
	vec3  lightColor   = u_spotLightParams[pointer + 0].xyz;
	float phi          = u_spotLightParams[pointer + 0].w;
	vec3  lightPos     = u_spotLightParams[pointer + 1].xyz;
	vec3  spotDir      = normalize(u_spotLightParams[pointer + 2].xyz);
	float radius       = u_spotLightParams[pointer + 3].x;
	float innerCutOff  = u_spotLightParams[pointer + 3].y;
	float outerCutOff  = u_spotLightParams[pointer + 3].z;
	
	vec3 lightDir = normalize(lightPos - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(normalDir, viewDir), 0.0);
	float NdotL = max(dot(normalDir, lightDir), 0.0);
	float NdotH = max(dot(normalDir, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(lightPos - worldPos);
	
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (radius * radius));
	vec3 radiance = lightColor * phi * INV_PI * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, F0);
	float NDF = DistributionGGX(NdotH, roughness);
	float Vis = Visibility(NdotV, NdotL, roughness);
	vec3 dirCookTorrance = Fre * NDF * Vis * u_doDirSpecular;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), metallic);
	
	float theta = dot(lightDir, normalize(-spotDir));
	float deno  = innerCutOff - outerCutOff;
	float spotClamp = saturate((theta - outerCutOff) / deno);
	
	return (KD * dirLambert + dirCookTorrance) * radiance * NdotL * spotClamp;
}
#endif

#if defined(DIRECTIONAL_LIGHT_LENGTH)
vec3 CalcDirectionalLight(int pointer, vec3 viewDir, vec3 normalDir, float roughness, float metallic, vec3 F0, vec3 dirLambert) { 
	// struct {
	// 	struct { float m_lightColor[3], m_intensity; };
	// 	struct { float m_lightDirection[3], m_unused0; };
	// } m_lights[MAX_LIGHT_COUNT];
	 
	vec3  lightColor = u_directionalLightParams[pointer + 0].xyz;
	float intensity  = u_directionalLightParams[pointer + 0].w;
	vec3  lightDir   = -normalize(u_directionalLightParams[pointer + 1].xyz);
	
	vec3 harfDir = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(normalDir, viewDir), 0.0);
	float NdotL = max(dot(normalDir, lightDir), 0.0);
	float NdotH = max(dot(normalDir, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	vec3  Fre = FresnelSchlick(HdotV, F0);
	float NDF = DistributionGGX(NdotH, roughness);
	float Vis = Visibility(NdotV, NdotL, roughness);
	vec3 dirCookTorrance = Fre * NDF * Vis * u_doDirSpecular;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), metallic);
	
	return (KD * dirLambert + dirCookTorrance) * intensity * NdotL;
}
#endif

void main()
{
	// normalMap
	vec3 normalMap = normalize(texture2D(s_texNormal, v_texcoord0).xyz * 2.0 - 1.0);
	vec3 normalDir = normalize(mul(v_TBN, normalMap));
	
	vec3 viewDir  = normalize(u_cameraPos - v_worldPos);
	float NdotV = max(dot(normalDir, viewDir), 0.0);
	
	vec3  albedo    = texture2D(s_texBaseColor, v_texcoord0).xyz; // already toLinear in cpu
	vec3  orm       = texture2D(s_texORM, v_texcoord0).xyz;
	float occlusion = orm.x;
	float roughness = clamp(orm.y, 0.04, 1.0);
	float metallic  = orm.z;
	
	vec3 F0 = mix(vec3_splat(0.04), albedo, metallic);

	// ------------------------------------ Directional Light ----------------------------------------
	
	// Direct Diffuse BRDF
	vec3 dirLambert = albedo * INV_PI * u_doDirDiffuse;
	
	vec3 dirColor = vec3_splat(0.0);
	
#if defined(POINT_LIGHT_LENGTH)
	for(int lightIndex = 0; lightIndex < u_pointLightCount; ++lightIndex) {
		int pointer = int(lightIndex * d_pointLightStride);
		dirColor += CalcPointLight(pointer, v_worldPos, viewDir, normalDir, roughness, metallic, F0, dirLambert);
	}
#endif
	
#if defined(SPOT_LIGHT_LENGTH)
	for(int lightIndex = 0; lightIndex < u_spotLightCount; ++lightIndex) {
		int pointer = int(lightIndex * d_spotLightStride);
		dirColor += CalcSpotLight(pointer, v_worldPos, viewDir, normalDir, roughness, metallic, F0, dirLambert);
	}
#endif
	
#if defined(DIRECTIONAL_LIGHT_LENGTH)
	for(int lightIndex = 0; lightIndex < u_directionalLightCount; ++lightIndex) {
		int pointer = int(lightIndex * d_directionalLightStride);
		dirColor += CalcDirectionalLight(pointer, viewDir, normalDir, roughness, metallic, F0, dirLambert);
	}
#endif
	
	// ----------------------------------- Environment Light ----------------------------------------
	
	float mip = clamp(6.0 * roughness, 0.1, 6.0);
	
	// Environment Prefiltered Irradiance
	vec3 cubeNormalDir = normalize(fixCubeLookup(normalDir, mip, 256.0));
	vec3 envIrradiance = toLinear(textureCube(s_texCubeIrr, cubeNormalDir).xyz) * u_doEnvDiffuse;
	
	// Environment Specular BRDF
	vec2 lut = texture2D(s_texLUT, vec2(NdotV, 1.0 - roughness)).xy;
	vec3 envSpecularBRDF = (F0 * lut.x + lut.y) * u_doEnvSpecular;
	
	// Environment Specular Radiance
	vec3 reflectDir = normalize(reflect(-viewDir, normalDir));
	vec3 cubeReflectDir = normalize(fixCubeLookup(reflectDir, mip, 256.0));
	vec3 envRadiance = toLinear(textureCubeLod(s_texCube, cubeReflectDir, mip).xyz);
	
	// Occlusion
	float specularOcclusion = lerp(pow(occlusion, 4.0), 1.0, saturate(-0.3 + NdotV * NdotV));
	float horizonOcclusion = saturate(1.0 + 1.2 * dot(reflectDir, v_normal));
	horizonOcclusion *= horizonOcclusion;
	
	vec3 F = FresnelSchlick(NdotV, F0);
	vec3 KD = mix(1.0 - F, vec3_splat(0.0), metallic);
	
	vec3 envColor = KD * albedo * envIrradiance * occlusion + envSpecularBRDF * envRadiance * min(specularOcclusion, horizonOcclusion);
	
	// ------------------------------------ Fragment Color -----------------------------------------
	
	gl_FragColor = vec4(dirColor + envColor, 1.0);
}
