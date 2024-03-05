//-----------------------------------------------------------------------------------------//
// @brief Calculates the contribution of all light sources.                                //
//                                                                                         //
// vec3 CalculateLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF); //
//-----------------------------------------------------------------------------------------//

#include "../UniformDefines/U_Light.sh"

uniform vec4 u_lightCountAndStride;
uniform vec4 u_lightParams[LIGHT_LENGTH];

U_Light GetLightParams(int pointer) {
	// struct {
	//   /*0*/ struct { float type; vec3 position; };
	//   /*1*/ struct { float intensity; vec3 color; };
	//   /*2*/ struct { float range; vec3 direction; };
	//   /*3*/ struct { float radius; vec3 up; };
	//   /*4*/ struct { float width, height, lightAngleScale, lightAngleOffeset; };
	// }
	
	U_Light light;
	light.type              = u_lightParams[pointer + 0].x;
	light.position          = u_lightParams[pointer + 0].yzw;
	light.intensity         = u_lightParams[pointer + 1].x;
	light.color             = u_lightParams[pointer + 1].yzw;
	light.range             = u_lightParams[pointer + 2].x;
	light.direction         = u_lightParams[pointer + 2].yzw;
	light.radius            = u_lightParams[pointer + 3].x;
	light.up                = u_lightParams[pointer + 3].yzw;
	light.width             = u_lightParams[pointer + 4].x;
	light.height            = u_lightParams[pointer + 4].y;
	light.lightAngleScale   = u_lightParams[pointer + 4].z;
	light.lightAngleOffeset = u_lightParams[pointer + 4].w;
	return light;
}

// -------------------- point -------------------- //
//TODO add cartoon point light

// -------------------- Directional -------------------- //

vec3 CalculateDirectionalLight(U_Light light, Material material, vec3 worldNormal, vec3 viewDir, vec3 diffuseBRDF) {
// TODO : Remove this normalize in the future.
	vec3 lightDir = normalize(-light.direction);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = dot(worldNormal, viewDir);
	float NdotL = dot(worldNormal, lightDir);
	float NdotH = max(dot(material.normal, harfDir), 0.0);

	float halfLambert = 0.5 * NdotL + 0.5; // Half Lambert
    //Diffuse
	vec3 firstShadeColor = u_firstShadowColor.xyz;
	vec3 secondShadeColor = u_secondShadowColor.xyz;
	
	float firstShadowMask = saturate( 1.0 - (halfLambert - (u_dividLine.x - u_dividLine.y)) / u_dividLine.y); // albedo and 1st shadow
	vec3 baseColor = lerp (material.albedo, firstShadeColor, firstShadowMask);

	float secondShadowMask = saturate ( 1.0 - (halfLambert - (u_dividLine.z - u_dividLine.w)) / u_dividLine.w); // 1st shadow and 2st shadow
	vec3 finalBaseColor = lerp (material.albedo,lerp(firstShadeColor, secondShadeColor,secondShadowMask),firstShadowMask);

	// Specular
	float halfSpecular = 0.5 * NdotH + 0.5;
	vec3 specularMask = u_specular.y * lerp (1.0 - step(halfSpecular, (1.0 - pow(u_specular.x, 5.0))), pow(halfSpecular, exp2(lerp(11, 1, u_specular.x))), u_specular.w);
	vec3 specularColor = light.color * specularMask;
	vec3 specular = specularColor * ((1.0 - firstShadowMask) + (firstShadowMask * u_specular.z)) * light.intensity;

	// Rim
	//TODO Screen Space rim
	float f = 1.0 - saturate(NdotV);
	f = smoothstep(1.0 - u_rimLight.x,1.0,f);
	f = smoothstep(0,u_rimLight.y,f);
	vec3 rim = f *  u_rimLightColor.xyz * u_rimLight.z;

	return (rim + specular + finalBaseColor);
}

// -------------------- Calculate each light -------------------- //

vec3 CalculateLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	if (light.type == DIRECTIONAL_LIGHT)
	{
		color = CalculateDirectionalLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	else
	{
		color = vec3_splat(0.0);
	}
	return color;
}

vec3 CalculateLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_lightCountAndStride.x); ++lightIndex) {
		int pointer = int(lightIndex * u_lightCountAndStride.y);
		U_Light light = GetLightParams(pointer);
		color += CalculateLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}
