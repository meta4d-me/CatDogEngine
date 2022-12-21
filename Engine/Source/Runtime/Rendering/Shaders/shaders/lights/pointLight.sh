#if defined(POINT_LIGHT_LENGTH)

struct PointLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
};

uniform vec4 u_pointLightParams[POINT_LIGHT_LENGTH];

PointLight GetPointLightParams(int pointer) {
	// struct {
	//  /*0*/ struct { float color[3], intensity; };
	//  /*1*/ struct { float position[3], range; };
	// }
	PointLight light;
	light.color     = u_pointLightParams[pointer + 0].xyz;
	light.intensity = u_pointLightParams[pointer + 0].w;
	light.position  = u_pointLightParams[pointer + 1].xyz;
	light.range     = u_pointLightParams[pointer + 1].w;
	return light;
}

vec3 CalculatePointLight(PointLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(light.position - worldPos);
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
	vec3 radiance = light.color * light.intensity * 0.25 * INV_PI * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = Fre * NDF * Vis;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	return (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

vec3 CalculatePointLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_pointLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_pointLightStride[0].x;
		PointLight light = GetPointLightParams(pointer);
		color += CalculatePointLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
