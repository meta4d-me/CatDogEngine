// BRDF Functions

// Fresnel
vec3 FresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Distribution
float DistributionGGX(float NdotH, float rough) {
	// a = rough^2
	float a2 = pow(rough, 4);
	float denom_inv = 1.0 / (NdotH * NdotH * (a2 - 1.0) + 1.0);
	return a2 * CD_PI_INV * denom_inv * denom_inv;
}

float Visibility_HighQuality(float NdotV, float NdotL, float rough) {
	// BRDF = (F * D * G) / (4 * NdotV * NdotL) = F * D * V
	// V = G / (4 * NdotV * NdotL)
	// = 0.5 / (NdotL * sqrt(a2 + (1 - a2) * NdotV^2) + NdotV * sqrt(a2 + (1 - a2) * NdotL^2))
	
	// rough = (rough + 1) / 2, by Disney
	// a = rough^2
	float a2 = pow((rough + 1.0) * 0.5, 4);
	float lambda_v = NdotL * sqrt(a2 + (1.0 - a2) * NdotV * NdotV);
	float lambda_l = NdotV * sqrt(a2 + (1.0 - a2) * NdotL * NdotL);
	
	return 0.5 / (lambda_v + lambda_l);
}

float Visibility_LowQuality(float NdotV, float NdotL, float rough) {
	// BRDF = (F * D * G) / (4 * NdotV * NdotL) = F * D * V
	// V = G / (4 * NdotV * NdotL)
	// = 1 / ((NdotV * (2 - a) + a) * (NdotL * (2 - a) + a))
	
	// rough = (rough + 1) / 2, by Disney
	// a = rough^2
	float a = (rough + 1.0) * (rough + 1.0) * 0.25;
	float g1_v_inv = NdotV * (2.0 - a) + a;
	float g1_l_inv = NdotL * (2.0 - a) + a;
	
	return 1.0 / (g1_v_inv * g1_l_inv);
}

// Geometry
float Visibility(float NdotV, float NdotL, float rough) {
	// TODO : Wrap them in macros after we've collected enough approximate / exact formulas.
	return Visibility_LowQuality(NdotV, NdotL, rough);
}