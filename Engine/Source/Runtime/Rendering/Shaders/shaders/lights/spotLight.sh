#if defined(SPOT_LIGHT_LENGTH)

struct SpotLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
	
	vec3 direction;
	float lightAngleScale;
	float lightAngleOffeset;
};

uniform vec4 u_spotLightParams[SPOT_LIGHT_LENGTH];

SpotLight GetSpotLightParams(int pointer) {
	// struct {
	//   /*0*/ struct { float color[3], intensity; };
	//   /*1*/ struct { float position[3], range; };
	//   /*2*/ struct { float direction[3], lightAngleScale; };
	//   /*3*/ struct { float lightAngleOffeset, unused0, unused1, unused2; };
	// }
	SpotLight light;
	light.color             = u_spotLightParams[pointer + 0].xyz;
	light.intensity         = u_spotLightParams[pointer + 0].w;
	light.position          = u_spotLightParams[pointer + 1].xyz;
	light.range             = u_spotLightParams[pointer + 1].w;
	light.direction         = normalize(u_spotLightParams[pointer + 2].xyz);
	light.lightAngleScale   = u_spotLightParams[pointer + 2].w;
	light.lightAngleOffeset = u_spotLightParams[pointer + 3].x;
	return light;
}

vec3 CalculateSpotLight(SpotLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(light.position - worldPos);
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
	attenuation *= GetAngleAtt(lightDir, -light.direction, light.lightAngleScale, light.lightAngleOffeset);
	vec3 radiance = light.color * light.intensity * INV_PI * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = Fre * NDF * Vis;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	return (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

vec3 CalculateSpotLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_spotLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_spotLightStride[0].x;
		SpotLight light = GetSpotLightParams(pointer);
		color += CalculateSpotLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
