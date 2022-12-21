#if defined(DIRECTIONAL_LIGHT_LENGTH)

struct DirectionalLight {
	vec3 color;
	// Irradiance/Illuminance in Luxes
	float intensity;
	vec3 direction;
};

uniform vec4 u_directionalLightParams[DIRECTIONAL_LIGHT_LENGTH];

DirectionalLight GetDirectionalLightParams(int pointer) {
	// struct {
	//   /*0*/ struct { float color[3], intensity; };
	//   /*1*/ struct { float direction[3], unused0; };
	// }
	DirectionalLight light;
	light.color     = u_directionalLightParams[pointer + 0].xyz;
	light.intensity = u_directionalLightParams[pointer + 0].w;
	light.direction = normalize(u_directionalLightParams[pointer + 1].xyz);
	return light;
}

vec3 CalculateDirectionalLight(DirectionalLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 lightDir = -light.direction;
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = Fre * NDF * Vis;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	vec3 irradiance = light.color * light.intensity;
	return (KD * diffuseBRDF + specularBRDF) * irradiance * NdotL;
}

vec3 CalculateDirectionalLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_directionalLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_directionalLightStride[0].x;
		DirectionalLight light = GetDirectionalLightParams(pointer);
		color += CalculateDirectionalLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
