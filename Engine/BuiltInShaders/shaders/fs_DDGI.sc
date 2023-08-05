$input v_worldPos, v_normal, v_texcoord0, v_TBN

#define IBL

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/DDGI.sh"
#include "../common/Envirnoment.sh"
#include "../common/Light.sh"

vec3 GetDirectional(Material material, vec3 worldPos, vec3 viewDir) {
	vec3 diffuseBRDF = material.albedo * CD_INV_PI;
	return CalculateLights(material, worldPos, viewDir, diffuseBRDF);
}

vec3 GetEnvironment(Material material, vec3 worldPos, vec3 viewDir, vec3 normal) {
	vec3 envDiffuseIrradiance = GetDDGIIrradiance(worldPos, normal, viewDir);
	vec3 ddgiRadiance = material.albedo * vec3_splat(CD_INV_PI) * envDiffuseIrradiance;
	vec3 iblRadiance = GetIBL(material, normal, viewDir);
	return material.metallic * iblRadiance + (1.0 - material.metallic) * ddgiRadiance;
}

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	material.roughness = 1.0 - material.roughness;
	
	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	
	vec3 dirColor = GetDirectional(material, v_worldPos, viewDir);
	vec3 envColor = GetEnvironment(material, v_worldPos, viewDir, v_normal);
	vec3 emiColor = material.emissive;
	
	gl_FragColor = vec4(dirColor + envColor + emiColor, 1.0);
}
