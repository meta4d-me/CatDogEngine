#if defined(RECTANGLE_LIGHT_LENGTH)

struct RectangleLight {
	vec3 color;
	// flux/power in lumens
	float intensity;
	vec3 position;
	float range;
	
	vec3 direction;
	float width;
	vec3 up;
	float height;
};

uniform vec4 u_rectangleLightParams[RECTANGLE_LIGHT_LENGTH];

RectangleLight GetRectangleLightParams(int pointer) {
	//struct {
	//  /*0*/ struct { float color[3], intensity; };
	//  /*1*/ struct { float position[3], range; };
	//  /*2*/ struct { float direction[3], width; };
	//  /*3*/ struct { float up[3], height; };
	//}
	RectangleLight light;
	light.color     = u_rectangleLightParams[pointer + 0].xyz;
	light.intensity = u_rectangleLightParams[pointer + 0].w;
	light.position  = u_rectangleLightParams[pointer + 1].xyz;
	light.range     = u_rectangleLightParams[pointer + 1].w;
	light.direction = normalize(u_rectangleLightParams[pointer + 2].xyz);
	light.width     = u_rectangleLightParams[pointer + 2].w;
	light.up        = normalize(u_rectangleLightParams[pointer + 3].xyz);
	light.height    = u_rectangleLightParams[pointer + 3].w;
	return light;
}

vec3 CalculateRectangleDiffuse(RectangleLight light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
	vec3 fragToLight = light.position - worldPos;
	vec3 lightDir = normalize(fragToLight);
	float sqrDist = dot(fragToLight, fragToLight);
	
	vec3 lightLeft = normalize(cross(light.direction, light.up));
	float halfWidth = light.width * 0.5;
	float halfHeight = light.height * 0.5;
	vec3 p0 = light.position + lightLeft * -halfWidth + light.up *  halfHeight;
	vec3 p1 = light.position + lightLeft * -halfWidth + light.up * -halfHeight;
	vec3 p2 = light.position + lightLeft *  halfWidth + light.up * -halfHeight;
	vec3 p3 = light.position + lightLeft *  halfWidth + light.up *  halfHeight;
	
	// Get the solid angle of rectangle light.
	float solidAngle = RectangleSolidAngle(worldPos, p0, p1, p2, p3);
	
	float averageCosine = 0.2 * (
		saturate(dot(normalize(p0 - worldPos), material.normal)) +
		saturate(dot(normalize(p1 - worldPos), material.normal)) +
		saturate(dot(normalize(p2 - worldPos), material.normal)) +
		saturate(dot(normalize(p3 - worldPos), material.normal)) +
		saturate(dot(normalize(light.position - worldPos), material.normal)));
	
	vec3 radiance = light.color * light.intensity / (light.width * light.height * PI);
	return diffuseBRDF * solidAngle * radiance * averageCosine * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateRectangleSpecular(RectangleLight light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
	vec3 reflectDir = normalize(reflect(viewDir, material.normal));
	
	// ReflectDir intersect with rectangle plane.
	float t = dot((light.position - worldPos), light.direction) / dot(reflectDir, light.direction);
	vec3 RIntersectP = worldPos + t * reflectDir;
	
	// Get the closest point on areal light with the reflect direction.
	vec3 offset = RIntersectP - light.position;
	float offsetLen2 = dot(offset, offset);
	vec3 horizontal = normalize(cross(light.direction, light.up));
	float cosTheta = dot(horizontal, offset / sqrt(offsetLen2));
	float cos2Theta = cosTheta * cosTheta;
	float tanPhi = light.height / light.width;
	float cos2Phi = 1.0 / (1.0 + tanPhi * tanPhi);
	float distToBorder2;
	if (cos2Theta > cos2Phi) {
		float distToBorder = light.width / 2.0 / cosTheta;
		distToBorder2 = distToBorder * distToBorder;
	}
	else {
		float sin2Theta = 1.0 - cos2Theta;
		distToBorder2 = light.height * light.height / 4.0 / sin2Theta;
	}
	vec3 d0 = -light.direction;
	vec3 d1 = -normalize(light.position - worldPos);
	vec3 d2 = reflectDir;
	float curR2 = saturate(offsetLen2 / distToBorder2) * distToBorder2;
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
	vec3 radiance = light.color * light.intensity / (light.width * light.height * PI) * attenuation;
	
	return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateRectangleLight(RectangleLight light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateRectangleSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateRectangleDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

vec3 CalculateRectangleLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_rectangleLightCount[0].x); ++lightIndex) {
		int pointer = lightIndex * u_rectangleLightStride[0].x;
		RectangleLight light = GetRectangleLightParams(pointer);
		color += CalculateRectangleLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	return color;
}

#endif
