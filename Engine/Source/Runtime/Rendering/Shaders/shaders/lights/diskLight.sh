#if defined(DISK_LIGHT_LENGTH)

struct DiskLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
	
	vec3 direction;
	float radius;
	float lightAngleScale;
	float lightAngleOffeset;
};

uniform vec4 u_diskLightParams[DISK_LIGHT_LENGTH];

DiskLight GetDiskLightParams(int pointer) {
	//struct {
	//  /*0*/ struct { float color[3], intensity; };
	//  /*1*/ struct { float position[3], range; };
	//  /*2*/ struct { float direction[3], radius; };
	//}
	DiskLight light;
	light.color             = u_diskLightParams[pointer + 0].xyz;
	light.intensity         = u_diskLightParams[pointer + 0].w;
	light.position          = u_diskLightParams[pointer + 1].xyz;
	light.range             = u_diskLightParams[pointer + 1].w;
	light.direction         = normalize(u_diskLightParams[pointer + 2].xyz);
	light.radius            = u_diskLightParams[pointer + 2].w;
	return light;
}

float cot(float x) { return cos(x) / sin(x); }
float acot(float x) { return atan(1.0 / x); }

vec3 CalculateDiskDiffuse(DiskLight light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
	vec3 fragToLight = light.position - worldPos;
	vec3 lightDir = normalize(fragToLight);
	float sqrDist = dot(fragToLight, fragToLight);
	
	// Get the form factor of areal light.
	float h = length(light.position - worldPos);
	float r = light.radius;
	float theta = acos(dot(material.normal, lightDir));
	float H = h / r;
	float H2 = H * H;
	float X = pow((1.0 - H2 * cot(theta) * cot(theta)), 0.5);
	float formFactor = 0.0;
	if(theta < acot(1.0 / H)) {
		formFactor = (1.0 / (1.0 + H2)) * cos(theta);
	}
	else {
		formFactor = -H * X * sin(theta) / (PI * (1.0 + H2)) +
		(1.0 / PI) * atan(X * sin(theta) / H) +
		cos(theta) * (PI - acos(H * cot(theta))) / (PI * (1.0 + H2));
	}
	formFactor = saturate(formFactor);
	
	vec3 radiance = light.color * light.intensity / (light.radius * light.radius * PI2);
	return diffuseBRDF * radiance * PI * formFactor * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateDiskSpecular(DiskLight light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
	vec3 reflectDir = normalize(reflect(viewDir, material.normal));
	
	// ReflectDir intersect with disk plane.
	float t = dot((light.position - worldPos), light.direction) / dot(reflectDir, light.direction);
	vec3 RIntersectP = worldPos + t * reflectDir;
	
	// Get the closest point on areal light with the reflect direction.
	vec3 d0 = -light.direction;
	vec3 d1 = normalize(worldPos - light.position);
	vec3 d2 = reflectDir;
	vec3 offset = RIntersectP - light.position;
	float r2 = light.radius * light.radius;
	float curR2 = saturate(dot(offset, offset) / r2) * r2;
	float k = -dot(d1, d0) / dot(d2, d0);
	vec3 d1kd2 = d1 + k * d2;
	float a2 = curR2 / dot(d1kd2, d1kd2);
	float a = sqrt(a2);
	vec3 d3 = sign(dot(-reflectDir, d0)) * a * d1kd2;
	vec3 closestPoint = light.position + d3;
	
	vec3 fragToClosest = closestPoint - worldPos;
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
	vec3 radiance = light.color * light.intensity / (light.radius * light.radius * PI2) * attenuation;
	
	return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateDiskLight(DiskLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateDiskSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateDiskDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

vec3 CalculateDiskLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_diskLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_diskLightStride[0].x;
		DiskLight light = GetDiskLightParams(pointer);
		color += CalculateDiskLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
