#if defined(TUBE_LIGHT_LENGTH)

struct TubeLight {
	vec3 color;
	// Radiant flux/Luminous flux in lumens
	float intensity;
	vec3 position;
	float range;
	
	vec3 direction;
	float radius;
	float width;
};

uniform vec4 u_tubeLightParams[TUBE_LIGHT_LENGTH];

TubeLight GetTubeLightParams(int pointer) {
	//struct {
	//  /*0*/ struct { float color[3], intensity; };
	//  /*1*/ struct { float position[3], range; };
	//  /*2*/ struct { float direction[3], radius; };
	//  /*3*/ struct { float width, unused0, unused1, unused2; };
	//}
	TubeLight light;
	light.color     = u_tubeLightParams[pointer + 0].xyz;
	light.intensity = u_tubeLightParams[pointer + 0].w;
	light.position  = u_tubeLightParams[pointer + 1].xyz;
	light.range     = u_tubeLightParams[pointer + 1].w;
	light.direction = normalize(u_tubeLightParams[pointer + 2].xyz);
	light.radius    = u_tubeLightParams[pointer + 2].w;
	light.width     = u_tubeLightParams[pointer + 3].x;
	return light;
}

// Return the closest point on the line (without limit)
vec3 closestPointOnLine(vec3 a, vec3 b, vec3 c) {
	vec3 ab = b - a;
	float t = dot(c - a, ab) / dot(ab , ab);
	return a + t * ab;
}

// Return the closest point on the segment (with limit)
vec3 closestPointOnSegment(vec3 a, vec3 b, vec3 c) {
	vec3 ab = b - a;
	float t = dot(c - a, ab) / dot(ab , ab);
	return a + saturate(t) * ab;
}

vec3 CalculateTubeDiffuse(TubeLight light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
	vec3 fragToLight = light.position - worldPos;
	float sqrDist = dot(fragToLight, fragToLight);
	
	// The sphere is placed at the nearest point on the segment.
	// The rectangular plane is define by the following orthonormal frame:
	float halfWidth = light.width * 0.5;
	vec3 left = light.direction;
	vec3 P0 = light.position - left * halfWidth;
	vec3 P1 = light.position + left * halfWidth;
	vec3 forward = normalize(worldPos - closestPointOnLine(P0, P1, worldPos));
	vec3 up = normalize(cross(left, forward));
	vec3 p0 = light.position - left * halfWidth + light.radius * up;
	vec3 p1 = light.position - left * halfWidth - light.radius * up;
	vec3 p2 = light.position + left * halfWidth - light.radius * up;
	vec3 p3 = light.position + left * halfWidth + light.radius * up;
	
	// Get the solid angle of rectangle light.
	float solidAngle = RectangleSolidAngle(worldPos, p0, p1, p2, p3);
	
	float averageCosine = 0.2 * (
		saturate(dot(normalize(p0 - worldPos), material.normal)) +
		saturate(dot(normalize(p1 - worldPos), material.normal)) +
		saturate(dot(normalize(p2 - worldPos), material.normal)) +
		saturate(dot(normalize(p3 - worldPos), material.normal)) +
		saturate(dot(normalize(light.position - worldPos), material.normal)));
	
	vec3 diffuseColor = vec3_splat(0.0);
	vec3 radiance = light.color * light.intensity * INV_PI /
		(2.0 * PI * light.radius * light.width + 4.0 * PI * light.radius * light.radius);
	diffuseColor += diffuseBRDF * solidAngle * radiance * averageCosine;
	
	// We then add the contribution of the sphere.
	vec3 spherePosition = closestPointOnSegment(P0, P1, worldPos);
	vec3 sphereFragToLight = spherePosition - worldPos;
	vec3 sphereLightDir = normalize(sphereFragToLight);
	float sphereSqrDist = dot(sphereFragToLight, sphereFragToLight);
	float sphereFormFactor = ((light.radius * light.radius) / sphereSqrDist) *
		saturate(dot(sphereLightDir, material.normal));
	sphereFormFactor = saturate(sphereFormFactor);
	
	diffuseColor += diffuseBRDF * radiance * PI * sphereFormFactor;
	return diffuseColor;
}

vec3 CalculateTubeSpecular(TubeLight light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
	vec3 reflectDir = normalize(reflect(viewDir, material.normal));
	
	float halfWidth = light.width * 0.5;
	vec3 left = light.direction;
	vec3 P0 = light.position - left * halfWidth;
	vec3 P1 = light.position + left * halfWidth;
	vec3 L0 = P0 - worldPos;
	vec3 L1 = P1 - worldPos;
	vec3 Ld = L1 - L0;
	float t = (dot(reflectDir, L0) * dot(reflectDir, Ld) - dot(L0, Ld)) /
		(dot(Ld, Ld) - pow(dot(reflectDir, Ld), 2.0));
	// A more precise function :
	//float t = (dot(L0, Ld) * dot(reflectDir, L0) - dot(L0, L0) * dot(reflectDir, Ld)) /
	//	(dot(L0, Ld) * dot(reflectDir, Ld) - dot(Ld, Ld) * dot(reflectDir, L0));
	vec3 fragToTight = L0 + saturate(t) * Ld;
	
	vec3 lightDir = normalize(fragToTight);
	vec3 harfDir  = normalize(lightDir + viewDir);
	float distance = length(fragToTight);
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
	vec3 radiance = light.color * light.intensity * INV_PI /
		(2.0 * PI * light.radius * light.width + 4.0 * PI * light.radius * light.radius) * attenuation;
	
	return specularBRDF * radiance * NdotL;
}

vec3 CalculateTubeLight(TubeLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateTubeSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateTubeDiffuse(light, material, worldPos, diffuseBRDF);
	return max(color, vec3_splat(0.0));
}

vec3 CalculateTubeLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_tubeLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_tubeLightStride[0].x;
		TubeLight light = GetTubeLightParams(pointer);
		color += CalculateTubeLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
