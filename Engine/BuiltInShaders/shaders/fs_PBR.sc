$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/LightSource.sh"
#include "../common/Envirnoment.sh"

uniform vec4 u_emissiveColor;

vec3 GetDirectional(Material material, vec3 worldPos, vec3 viewDir) {
	vec3 diffuseBRDF = material.albedo * CD_INV_PI;
	return CalculateLights(material, worldPos, viewDir, diffuseBRDF);
}

vec3 GetEnvironment(Material material, vec3 worldPos, vec3 viewDir, vec3 normal) {
	return GetIBL(material, normal, viewDir) + GetATM(material, worldPos);
}

float GetViewLinearDepth(float depth){
    depth = depth * 2.0 - 1.0; 
	float NEAR = 0.1; 
	float FAR = 2000.0;
	return (2.0 * NEAR * FAR) / (FAR + NEAR - depth * (FAR - NEAR));
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
	vec3 dirColor = GetDirectional(material, v_worldPos, viewDir);
	
	// Environment Light
	vec3 envColor = GetEnvironment(material, v_worldPos, viewDir, v_normal);
	
	// Emissive
	vec3 emiColor = material.emissive * u_emissiveColor.xyz;
	
	// Fragment Color
	gl_FragData[0] = vec4(dirColor + envColor + emiColor, 1.0);
	gl_FragData[1] = vec4(emiColor, 1.0);

	vec3 viewPos = mul(u_view,vec4(v_worldPos,1.0)).xyz * vec3(1,1,-1);
	vec3 viewNormal = normalize(mul(vec4(v_normal,0.0),u_invView).xyz) * vec3(1,1,-1);
	float depth = GetViewLinearDepth(gl_FragCoord.z);

	gl_FragData[2] = vec4(viewPos,depth);
	gl_FragData[3] = vec4(viewNormal,1.0);
	
	// Post-processing will be used in the last pass.
}
