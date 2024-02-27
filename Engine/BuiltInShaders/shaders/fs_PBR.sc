$input v_worldPos, v_normal, v_texcoord0, v_TBN, v_color0

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/LightSource.sh"
#include "../common/Envirnoment.sh"

uniform vec4 u_emissiveColorAndFactor;
uniform vec4 u_cameraNearFarPlane;

vec3 GetDirectional(Material material, vec3 worldPos, vec3 viewDir, float csmDepth) {
	vec3 diffuseBRDF = material.albedo * CD_PI_INV;
	return CalculateLights(material, worldPos, viewDir, diffuseBRDF, csmDepth);
}

vec3 GetEnvironment(Material material, vec3 worldPos, vec3 viewDir, vec3 normal) {
	return GetIBL(material, normal, viewDir) + GetATM(material, worldPos);
}

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	if (material.opacity < u_alphaCutOff.x) {
		discard;
	}
	
	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	
	// Directional Light
	float csmDepth = (v_color0.z - u_cameraNearFarPlane.x) / (u_cameraNearFarPlane.y - u_cameraNearFarPlane.x);
	vec3 dirColor = GetDirectional(material, v_worldPos, viewDir, csmDepth);
	
	// Environment Light
	vec3 envColor = GetEnvironment(material, v_worldPos, viewDir, v_normal);
	
	// Emissive
	vec3 emiColor = material.emissive * u_emissiveColorAndFactor.xyz * vec3_splat(u_emissiveColorAndFactor.w);
	
	// Fragment Color
	gl_FragData[0] = vec4(dirColor + envColor + emiColor, 1.0);
	gl_FragData[1] = vec4(emiColor, 1.0);
	
	// Post-processing will be used in the last pass.
}
