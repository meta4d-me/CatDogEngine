#if defined(SPHERE_LIGHT_LENGTH)

struct SphereLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
	
	float radius;
};

uniform vec4 u_sphereLightParams[SPHERE_LIGHT_LENGTH];

SphereLight GetSphereLightParams(int pointer) {
	// struct {
	//   /*0*/ struct { float color[3], intensity; };
	//   /*1*/ struct { float position[3], range; };
	//   /*2*/ struct { float radius, unused0, unused1, unused2; };
	// }
	SphereLight light;
	light.color     = u_sphereLightParams[pointer + 0].xyz;
	light.intensity = u_sphereLightParams[pointer + 0].w;
	light.position  = u_sphereLightParams[pointer + 1].xyz;
	light.range     = u_sphereLightParams[pointer + 1].w;
	light.radius    = u_sphereLightParams[pointer + 2].x;
	return light;
}

vec3 CalculateSphereDiffuse(SphereLight light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
	vec3 fragToLight = light.position - worldPos;
	vec3 lightDir = normalize(fragToLight);
	float sqrDist = dot(fragToLight, fragToLight);
	
	// Get the form factor of areal light.
	float beta = acos(dot(material.normal, lightDir));
	float H = sqrt(sqrDist);
	float h = H / light.radius;
	float x = sqrt(h * h - 1.0);
	float y = -x * (1.0 / tan(beta));
	float formFactor = 0.0;
	if(h * cos(beta) > 1.0) {
		formFactor = cos(beta) / (h * h);
	}
	else {
		formFactor = (1.0 / (PI * h * h)) *
		(cos(beta) * acos(y) - x * sin(beta) * sqrt(1.0 - y * y)) +
		(1.0 / PI) * atan(sin(beta) * sqrt(1.0 - y * y) / x);
	}
	formFactor = saturate(formFactor);
	
	vec3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * PI2);
	return diffuseBRDF * radiance * PI * formFactor;
}

vec3 CalculateSphereSpecular(SphereLight light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
	vec3 reflectDir = normalize(reflect(viewDir, material.normal));
	
	// Get the closest point on areal light with the reflect direction.
	vec3 fragToLight = light.position - worldPos;
	vec3 centerToRay = dot(fragToLight, reflectDir) * reflectDir - fragToLight;
	vec3 fragToClosest = fragToLight + centerToRay * saturate(light.radius / length(centerToRay));
	
	vec3 lightDir = normalize(fragToClosest);
	vec3 harfDir  = normalize(lightDir + viewDir);
	float distance = length(fragToClosest);
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	// Get the specular BRDF.
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = saturate(Fre * NDF * Vis);
	KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
	vec3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * PI2) * attenuation;
	
	return specularBRDF * radiance * NdotL;
}

vec3 CalculateSphereLight(SphereLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateSphereSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateSphereDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

vec3 CalculateSphereLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_sphereLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_sphereLightStride[0].x;
		SphereLight light = GetSphereLightParams(pointer);
		color += CalculateSphereLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
