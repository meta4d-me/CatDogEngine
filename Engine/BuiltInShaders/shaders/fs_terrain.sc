#define IBL
$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/LightSource.sh"
#include "../common/Envirnoment.sh"

#include "../UniformDefines/U_Terrain.sh"

uniform vec4 u_emissiveColor;

SAMPLER2D(s_texSnow, TERRAIN_TOP_ALBEDO_MAP_SLOT);
SAMPLER2D(s_texRock, TERRAIN_MEDIUM_ALBEDO_MAP_SLOT);
SAMPLER2D(s_texGrass, TERRAIN_BOTTOM_ALBEDO_MAP_SLOT);

vec3 GetDirectional(Material material, vec3 worldPos, vec3 viewDir) {
	vec3 diffuseBRDF = material.albedo * CD_INV_PI;
	return CalculateLights(material, worldPos, viewDir, diffuseBRDF);
}

vec3 GetEnvironment(Material material, vec3 normal, vec3 viewDir) {
	return GetIBL(material, normal, viewDir);
}

vec4 CalcTexColor(vec3 pos, vec2 texcoord)
{
   vec4 TexColor;// = texture2D(s_texGrass, texcoord);
   float Height = pos.y;
   float height0 = 5;
   float height1 = 15;
   float height2 = 25;
   if (Height < height0) 
   {
      TexColor = texture2D(s_texGrass, texcoord);
   } 
   else if (Height < height1) 
   {
      vec4 Color0 = texture2D(s_texGrass, texcoord);
      vec4 Color1 = texture2D(s_texRock, texcoord);
      float Delta = height1 - height0;
      float Factor = (Height - height0) / Delta;
      TexColor = mix(Color0, Color1, Factor);
   } 
   else if (Height <height2) 
   {
      vec4 Color0 = texture2D(s_texRock, texcoord);
      vec4 Color1 = texture2D(s_texSnow, texcoord);
      float Delta = height2 - height1;
      float Factor = (Height - height1) / Delta;
      TexColor = mix(Color0, Color1, Factor);
   } 
   else 
   {
      TexColor = texture2D(s_texSnow, texcoord);
   }

   return TexColor;  
}

void main()
{
	Material material = GetMaterial(v_texcoord0, v_normal, v_TBN);
	if (material.opacity < u_alphaCutOff.x) {
		discard;
	}

   material.albedo = CalcTexColor(v_worldPos, v_texcoord0);

	vec3 cameraPos = GetCamera().position.xyz;
	vec3 viewDir = normalize(cameraPos - v_worldPos);
	
	// Directional Light
	vec3 dirColor = GetDirectional(material, v_worldPos, viewDir);
	
	// Environment Light
	vec3 envColor = GetEnvironment(material, v_normal, viewDir);
	
	// Emissive
	vec3 emiColor = material.emissive * u_emissiveColor.xyz;
	
	// Fragment Color
	gl_FragColor = vec4(dirColor + envColor + emiColor, 1.0);//vec4(material.albedo, 1.0);
	// Post-processing will be used in the last pass.
}

