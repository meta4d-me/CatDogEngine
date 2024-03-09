//--------------------------------------------------------------------------------------------------------//
// @brief Calculates the contribution of all light sources.                                               //
//                                                                                                        //
// vec3 CalculateLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, float csmDepth) //
//--------------------------------------------------------------------------------------------------------//

#include "../UniformDefines/U_Light.sh"
#include "../UniformDefines/U_Shadow.sh"

uniform vec4 u_lightCountAndStride;
uniform vec4 u_lightParams[LIGHT_LENGTH];
uniform mat4 u_lightViewProjs[4*3];
uniform vec4 u_clipFrustumDepth;
uniform vec4 u_bias[3];//[LIGHT_NUM]

SAMPLERCUBE(s_texCubeShadowMap_1, SHADOW_MAP_CUBE_FIRST_SLOT);
SAMPLERCUBE(s_texCubeShadowMap_2, SHADOW_MAP_CUBE_SECOND_SLOT);
SAMPLERCUBE(s_texCubeShadowMap_3, SHADOW_MAP_CUBE_THIRD_SLOT);

U_Light GetLightParams(int pointer) {
	// struct {
	//   /*0*/ struct { float type; vec3 position; };
	//   /*1*/ struct { float intensity; vec3 color; };
	//   /*2*/ struct { float range; vec3 direction; };
	//   /*3*/ struct { float radius; vec3 up; };
	//   /*4*/ struct { float width, height, lightAngleScale, lightAngleOffeset; };
	//	 /*5*/ struct {	int shadowType, lightViewProjOffset, cascadeNum; float shadowBias; };
	//	 /*6*/ struct { vec4 frustumClips; };
	// }
	
	U_Light light;
	light.type              	= u_lightParams[pointer + 0].x;
	light.position          	= u_lightParams[pointer + 0].yzw;
	light.intensity         	= u_lightParams[pointer + 1].x;
	light.color             	= u_lightParams[pointer + 1].yzw;
	light.range             	= u_lightParams[pointer + 2].x;
	light.direction         	= u_lightParams[pointer + 2].yzw;
	light.radius            	= u_lightParams[pointer + 3].x;
	light.up                	= u_lightParams[pointer + 3].yzw;
	light.width             	= u_lightParams[pointer + 4].x;
	light.height            	= u_lightParams[pointer + 4].y;
	light.lightAngleScale   	= u_lightParams[pointer + 4].z;
	light.lightAngleOffeset 	= u_lightParams[pointer + 4].w;
	light.shadowType        	= asint(u_lightParams[pointer + 5].x);
	light.lightViewProjOffset	= asint(u_lightParams[pointer + 5].y);
	light.cascadeNum        	= asint(u_lightParams[pointer + 5].z);
	light.shadowBias        	= u_lightParams[pointer + 5].z;
	light.frustumClips      	= u_lightParams[pointer + 6];
	return light;
}

// -------------------- Utils -------------------- //

// Distance Attenuation
float SmoothDistanceAtt(float squaredDistance, float invSqrAttRadius) {
	float factor = squaredDistance * invSqrAttRadius;
	float smoothFactor = saturate(1.0 - factor * factor);
	return smoothFactor * smoothFactor;
}
float GetDistanceAtt(float sqrDist, float invSqrAttRadius) {
	float attenuation = 1.0 / (max(sqrDist , 0.0001));
	attenuation *= SmoothDistanceAtt(sqrDist, invSqrAttRadius);
	return attenuation;
}

// Angle Attenuation
float GetAngleAtt(vec3 lightDir, vec3 lightForward, float lightAngleScale, float lightAngleOffeset) {
	// On CPU
	// float lightAngleScale = 1.0f / max(0.001f, cosInner - cosOuter);
	// float lightAngleOffeset = -cosOuter * angleScale;
	
	// lightDir: Start from fragment, lightForward: Start from light source.
	float theta = dot(-lightDir, lightForward);
	float attenuation = saturate(theta * lightAngleScale + lightAngleOffeset);
	attenuation *= attenuation;
	return attenuation;
}

float cot(float x) {
	return cos(x) / sin(x);
}
float acot(float x) {
	return atan(1.0 / x);
}

float RectangleSolidAngle(vec3 worldPos , vec3 p0 , vec3 p1 ,vec3 p2 , vec3 p3) {
	vec3 v0 = p0 - worldPos;
	vec3 v1 = p1 - worldPos;
	vec3 v2 = p2 - worldPos;
	vec3 v3 = p3 - worldPos;
	vec3 n0 = normalize(cross(v0, v1));
	vec3 n1 = normalize(cross(v1, v2));
	vec3 n2 = normalize(cross(v2, v3));
	vec3 n3 = normalize(cross(v3, v0));
	float g0 = acos(dot(-n0, n1));
	float g1 = acos(dot(-n1, n2));
	float g2 = acos(dot(-n2, n3));
	float g3 = acos(dot(-n3, n0));
	return g0 + g1 + g2 + g3 - 2.0 * CD_PI;
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

// Return shadow map sampler according to light index
float textureIndex(int lightIndex, vec3 sampleVec){
	float closestDepth = 1.0;
	if(0 == lightIndex){
		closestDepth = textureCube(s_texCubeShadowMap_1, sampleVec).r; 
	}else if(1 == lightIndex){
		closestDepth = textureCube(s_texCubeShadowMap_2, sampleVec).r; 
	}else if(2 == lightIndex){
		closestDepth = textureCube(s_texCubeShadowMap_3, sampleVec).r; 
	}
	return closestDepth; 
}


// -------------------- Point -------------------- //

float CalculatePointShadow(vec3 fragPosWorldSpace, vec3 lightPosWorldSpace, float far_plane, int lightIndex) {
	vec3 lightToFrag = fragPosWorldSpace - lightPosWorldSpace;
	float currentDepth = length(lightToFrag);
    float bias = 0.05;

	// PCF shadow | Filter Size : 3x3 
	float shadow = 0.0;
	float samples = 3.0;
	float totalOffset = 0.03;
	float stepOffset = totalOffset / ((samples-1) * 0.5);
	for(float x = -totalOffset; x <= totalOffset; x += stepOffset)
	{
		for(float y = -totalOffset; y <= totalOffset; y += stepOffset)
		{
			for(float z = -totalOffset; z <= totalOffset; z += stepOffset)
			{
				float closestDepth = textureIndex(lightIndex, lightToFrag + vec3(x, y, z)); 
				closestDepth *= far_plane;
				shadow += step(closestDepth, currentDepth - bias); 
			}
		}
	}
	shadow /= (samples * samples * samples);

	return shadow;
}

vec3 CalculatePointLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, int lightIndex) {
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(light.position - worldPos);
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
	vec3 radiance = light.color * light.intensity * 0.25 * CD_PI_INV * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = Fre * NDF * Vis;
	
	vec3 KD = mix(vec3_splat(1.0) - Fre, vec3_splat(0.0), material.metallic);
	float shadow = CalculatePointShadow(worldPos, light.position, light.range, lightIndex);
	return (1 - shadow) * (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

// -------------------- Spot -------------------- //

float CalculateSpotShadow(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir,int lightViewProjOffset, int lightIndex){
    vec4 fragPosLightSpace = mul(u_lightViewProjs[lightViewProjOffset], vec4(fragPosWorldSpace, 1.0));
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	vec3 sampleVec = vec3(1.0, projCoords.y, -projCoords.x);
    float fragDepth = projCoords.z;
    
	// Calculate bias (based on depth map resolution and slope)
    float bias = max(0.001 * (1.0 - dot(normal, lightDir)), 0.00001);

	// PCF Filter Size : 3x3 
	float shadow = 0.0;
	float samples = 3.0;
	float totalOffset = 0.0015;
	float stepOffset = totalOffset / ((samples-1) * 0.5);
	for(float x = -totalOffset; x <= totalOffset; x += stepOffset){
		for(float y = -totalOffset; y <= totalOffset; y += stepOffset){
			float closestDepth = textureIndex(lightIndex, sampleVec + vec3(0, y, x));
			shadow += step(closestDepth, fragDepth - bias); 
		}
	}
	shadow /= (samples * samples);

    return shadow;
}

vec3 CalculateSpotLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, int lightIndex) {
	vec3 lightDir = normalize(light.position - worldPos);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = max(dot(material.normal, viewDir), 0.0);
	float NdotL = max(dot(material.normal, lightDir), 0.0);
	float NdotH = max(dot(material.normal, harfDir), 0.0);
	float HdotV = max(dot(harfDir, viewDir), 0.0);
	
	float distance = length(light.position - worldPos);
	float attenuation = GetDistanceAtt(distance * distance, 1.0 / (light.range * light.range));
	// TODO : Remove this normalize in the future.
	attenuation *= GetAngleAtt(lightDir, normalize(light.direction), light.lightAngleScale, light.lightAngleOffeset);
	vec3 radiance = light.color * light.intensity * CD_PI_INV * attenuation;
	
	vec3  Fre = FresnelSchlick(HdotV, material.F0);
	float NDF = DistributionGGX(NdotH, material.roughness);
	float Vis = Visibility(NdotV, NdotL, material.roughness);
	vec3 specularBRDF = Fre * NDF * Vis;
	
	vec3 KD = mix(1.0 - Fre, vec3_splat(0.0), material.metallic);
	float shadow = CalculateSpotShadow(worldPos, material.normal, lightDir, light.lightViewProjOffset, lightIndex);
	return (1.0 - shadow) * (KD * diffuseBRDF + specularBRDF) * radiance * NdotL;
}

// -------------------- Directional -------------------- //

float CalculateDirectionalShadow(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir, mat4 lightViewProj, int lightIndex, float num){
    vec4 fragPosLightSpace = mul(lightViewProj, vec4(fragPosWorldSpace, 1.0));
    vec3 projCoords = fragPosLightSpace.xyz/fragPosLightSpace.w;
    float currentDepth = projCoords.z;
    float bias = max(0.02 * (1.0 - dot(normal, lightDir)), 0.002);

	// PCF shadow | Filter Size : 3x3 
	vec3 sampleVec;
	if(num < 0.5) sampleVec = vec3(1.0, projCoords.y, -projCoords.x);
	else if(num > 0.5 && num < 1.5) sampleVec = vec3(-1.0, projCoords.y, projCoords.x);
	else if(num > 1.5 && num < 2.5) sampleVec = vec3(projCoords.x, 1.0, projCoords.y);
	else if(num > 2.5 && num < 3.5) sampleVec = vec3(projCoords.x, -1.0, projCoords.y);
	float shadow = 0.0;
	float samples = 3.0;
	float totalOffset = 0.0015;
	float stepOffset = totalOffset / ((samples-1) * 0.5);
	if(num < 1.5){
		for(float x = -totalOffset; x <= totalOffset; x += stepOffset){
    		for(float y = -totalOffset; y <= totalOffset; y += stepOffset){
				float closestDepth = textureIndex(lightIndex, sampleVec + vec3(0, y, x));
				shadow += step(closestDepth, currentDepth - bias); 
			}
		}
	}
	else{
		for(float x = -totalOffset; x <= totalOffset; x += stepOffset){
    		for(float y = -totalOffset; y <= totalOffset; y += stepOffset){
				float closestDepth = textureIndex(lightIndex, sampleVec + vec3(x, 0, y));
				shadow += step(closestDepth, currentDepth - bias); 
			}
		}
	}
	shadow /= (samples * samples);
	
    return shadow;
}

float CalculateCascadedDirectionalShadow(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir, float csmDepth, int lightViewProjOffset, int lightIndex){
	if(csmDepth > 0 && csmDepth <= u_clipFrustumDepth.x)
		return CalculateDirectionalShadow(fragPosWorldSpace, normal, lightDir, u_lightViewProjs[lightViewProjOffset], lightIndex, 0.0);
	else if(csmDepth > u_clipFrustumDepth.x && csmDepth <= u_clipFrustumDepth.y)
		return CalculateDirectionalShadow(fragPosWorldSpace, normal, lightDir, u_lightViewProjs[lightViewProjOffset+1], lightIndex, 1.0);
	else if(csmDepth > u_clipFrustumDepth.y && csmDepth <= u_clipFrustumDepth.z)
		return CalculateDirectionalShadow(fragPosWorldSpace, normal, lightDir, u_lightViewProjs[lightViewProjOffset+2], lightIndex, 2.0);
	else if(csmDepth > u_clipFrustumDepth.z && csmDepth <= 1)
		return CalculateDirectionalShadow(fragPosWorldSpace, normal, lightDir, u_lightViewProjs[lightViewProjOffset+3], lightIndex, 3.0);
	else
		return 1.0;
}

vec3 CalculateDirectionalLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, float csmDepth, int lightIndex) {
	// TODO : Remove this normalize in the future.
	vec3 lightDir = normalize(-light.direction);
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
	float shadow = CalculateCascadedDirectionalShadow(worldPos, material.normal, lightDir, csmDepth, light.lightViewProjOffset, lightIndex);
	return (1.0 - shadow) * (KD * diffuseBRDF + specularBRDF) * irradiance * NdotL;
}

vec3 CalculateCelDirectionalLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, float csmDepth, int lightIndex) {
// TODO : Remove this normalize in the future. and and shadow in cartoon renderer
	vec3 lightDir = normalize(-light.direction);
	vec3 harfDir  = normalize(lightDir + viewDir);
	
	float NdotV = dot(material.normal, viewDir);
	float NdotL = dot(material.normal, lightDir);
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


// -------------------- Sphere -------------------- //

vec3 CalculateSphereDiffuse(U_Light light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
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
		formFactor = (1.0 / (CD_PI * h * h)) *
		(cos(beta) * acos(y) - x * sin(beta) * sqrt(1.0 - y * y)) +
		CD_PI_INV * atan(sin(beta) * sqrt(1.0 - y * y) / x);
	}
	formFactor = saturate(formFactor);
	
	vec3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * CD_PI2);
	return diffuseBRDF * radiance * CD_PI * formFactor;
}

vec3 CalculateSphereSpecular(U_Light light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
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
	vec3 radiance = light.color * light.intensity / (4.0 * light.radius * light.radius * CD_PI2) * attenuation;
	
	return specularBRDF * radiance * NdotL;
}

vec3 CalculateSphereLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateSphereSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateSphereDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

// -------------------- Disk -------------------- //

vec3 CalculateDiskDiffuse(U_Light light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
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
		formFactor = -H * X * sin(theta) / (CD_PI * (1.0 + H2)) +
		CD_PI_INV * atan(X * sin(theta) / H) +
		cos(theta) * (CD_PI - acos(H * cot(theta))) / (CD_PI * (1.0 + H2));
	}
	formFactor = saturate(formFactor);
	
	vec3 radiance = light.color * light.intensity / (light.radius * light.radius * CD_PI2);
	return diffuseBRDF * radiance * CD_PI * formFactor * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateDiskSpecular(U_Light light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
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
	vec3 radiance = light.color * light.intensity / (light.radius * light.radius * CD_PI2) * attenuation;
	
	return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateDiskLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateDiskSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateDiskDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

// -------------------- Rectangle -------------------- //

vec3 CalculateRectangleDiffuse(U_Light light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
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
	
	vec3 radiance = light.color * light.intensity / (light.width * light.height * CD_PI);
	return diffuseBRDF * solidAngle * radiance * averageCosine * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateRectangleSpecular(U_Light light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
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
	vec3 radiance = light.color * light.intensity / (light.width * light.height * CD_PI) * attenuation;
	
	return specularBRDF * radiance * saturate(dot(light.direction, -lightDir));
}

vec3 CalculateRectangleLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateRectangleSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateRectangleDiffuse(light, material, worldPos, diffuseBRDF);
	return color;
}

// -------------------- Tube -------------------- //

vec3 CalculateTubeDiffuse(U_Light light, Material material, vec3 worldPos, vec3 diffuseBRDF) {
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
	vec3 radiance = light.color * light.intensity * CD_PI_INV /
		(2.0 * CD_PI * light.radius * light.width + 4.0 * CD_PI * light.radius * light.radius);
	diffuseColor += diffuseBRDF * solidAngle * radiance * averageCosine;
	
	// We then add the contribution of the sphere.
	vec3 spherePosition = closestPointOnSegment(P0, P1, worldPos);
	vec3 sphereFragToLight = spherePosition - worldPos;
	vec3 sphereLightDir = normalize(sphereFragToLight);
	float sphereSqrDist = dot(sphereFragToLight, sphereFragToLight);
	float sphereFormFactor = ((light.radius * light.radius) / sphereSqrDist) *
		saturate(dot(sphereLightDir, material.normal));
	sphereFormFactor = saturate(sphereFormFactor);
	
	diffuseColor += diffuseBRDF * radiance * CD_PI * sphereFormFactor;
	return diffuseColor;
}

vec3 CalculateTubeSpecular(U_Light light, Material material, vec3 worldPos, vec3 viewDir, out vec3 KD) {
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
	vec3 radiance = light.color * light.intensity * CD_PI_INV /
		(2.0 * CD_PI * light.radius * light.width + 4.0 * CD_PI * light.radius * light.radius) * attenuation;
	
	return specularBRDF * radiance * NdotL;
}

vec3 CalculateTubeLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF) {
	vec3 color = vec3_splat(0.0);
	vec3 KD = vec3_splat(1.0);
	color += CalculateTubeSpecular(light, material, worldPos, viewDir, KD);
	color += KD * CalculateTubeDiffuse(light, material, worldPos, diffuseBRDF);
	return max(color, vec3_splat(0.0));
}

// -------------------- Calculate each light -------------------- //

vec3 CalculateLight(U_Light light, Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, float csmDepth, int lightIndex) {
	vec3 color = vec3_splat(0.0);
	if (light.type == POINT_LIGHT)
	{
		color = CalculatePointLight(light, material, worldPos, viewDir, diffuseBRDF, lightIndex);
	}
	else if (light.type == SPOT_LIGHT)
	{
		color = CalculateSpotLight(light, material, worldPos, viewDir, diffuseBRDF, lightIndex);
	}
	else if (light.type == DIRECTIONAL_LIGHT)
	{
		color = CalculateDirectionalLight(light, material, worldPos, viewDir, diffuseBRDF, csmDepth, lightIndex);
	}
	else if (light.type == SPHERE_LIGHT)
	{
		color = CalculateSphereLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	else if (light.type == DISK_LIGHT)
	{
		color = CalculateDiskLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	else if (light.type == RECTANGLE_LIGHT)
	{
		color = CalculateRectangleLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	else if (light.type == TUBE_LIGHT)
	{
		color = CalculateTubeLight(light, material, worldPos, viewDir, diffuseBRDF);
	}
	else
	{
		color = vec3_splat(0.0);
	}
	return color;
}

vec3 CalculateCelLight(U_Light light, Material material, vec3 normal, vec3 viewDir, float csmDepth, int lightIndex) {
	vec3 color = vec3_splat(0.0);
	if (light.type == POINT_LIGHT)
	{
		color = vec3_splat(0.0); //add cartoon point light
	}
	else if (light.type == SPOT_LIGHT)
	{
		color = vec3_splat(0.0); //add cartoon spot light
	}
	else if (light.type == DIRECTIONAL_LIGHT)
	{
		color = CalculateCelDirectionalLight(light, material, normal, viewDir, csmDepth, lightIndex);
	}
	else
	{
		color = vec3_splat(0.0);
	}
	return color;
}

vec3 CalculateLights(Material material, vec3 worldPos, vec3 viewDir, vec3 diffuseBRDF, float csmDepth) {
	vec3 color = vec3_splat(0.0);
	for(int lightIndex = 0; lightIndex < int(u_lightCountAndStride.x); ++lightIndex) {
		int pointer = int(lightIndex * u_lightCountAndStride.y);
		U_Light light = GetLightParams(pointer);
		color += CalculateLight(light, material, worldPos, viewDir, diffuseBRDF, csmDepth, lightIndex);
	}
	return color;
}

vec3 CalculateCelLights(Material material, vec3 worldPos, vec3 viewDir, float csmDepth) {
	vec3 color = vec3_splat(0.0);
		for(int lightIndex = 0; lightIndex < int(u_lightCountAndStride.x); ++lightIndex) {
		int pointer = int(lightIndex * u_lightCountAndStride.y);
		U_Light light = GetLightParams(pointer);
		color += CalculateCelLight(light, material, worldPos, viewDir, csmDepth, lightIndex);
	}
	return color;
}
