$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_lightingColor, 0);
SAMPLER2D(s_bloomColor, 1);
uniform vec4 u_gamma;

vec3 ACES(vec3 color) {
	mat3 ACESInputMat  = mtxFromRows(vec3(0.59719,  0.35458,  0.04823), vec3( 0.07600, 0.90834,  0.01566), vec3( 0.02840,  0.13383, 0.83777));
	mat3 ACESOutputMat = mtxFromRows(vec3(1.60475, -0.53108, -0.07367), vec3(-0.10208, 1.10813, -0.00605), vec3(-0.00327, -0.07276, 1.07602));
	
	color      = mul(ACESInputMat, color);
	vec3 nom   = color * (color + 0.0245786) - 0.000090537;
	vec3 denom = color * (0.983729 * color + 0.4329510) + 0.238081;
	color      = nom / denom;
	color      = mul(ACESOutputMat, color);
	
	return color;
}

// Exposure
float ComputeEV100(float aperture ,float shutterTime ,float ISO) {
	// EV number is defined as:
	// 2^ EV_s = N^2 / t and EV_s = EV_100 + log2 (S /100)
	// This gives
	// EV_s = log2 (N^2 / t)
	// EV_100 + log2 (S /100) = log2 (N^2 / t)
	// EV_100 = log2 (N^2 / t) - log2 (S /100)
	// EV_100 = log2 (N^2 / t . 100 / S)
	
	return log2(aperture * aperture / shutterTime * 100.0 / ISO);
}

float ConvertEV100ToExposure(float EV100) {
	// Compute the maximum luminance possible with H_sbs sensitivity
	// maxLum = 78 / ( S * q ) * N^2 / t
	// = 78 / ( S * q ) * 2^ EV_100
	// = 78 / (100 * 0.65) * 2^ EV_100
	// = 1.2 * 2^ EV
	// Reference : http :// en. wikipedia . org / wiki / Film_speed
	
	return 1.0 / (1.2 * exp2(EV100));
}

void main()
{
	vec3 color = texture2D(s_lightingColor, v_texcoord0).rgb;
	vec3 bloom = texture2D(s_bloomColor, v_texcoord0).rgb;
	color += bloom;
	
	// Exposure
	color *= ConvertEV100ToExposure(0.0);
	
	// Tone Mapping
	color = ACES(color);
	
	// Gamma Correction
	color = pow(color, vec3_splat(u_gamma.x));
	
	gl_FragColor = vec4(color, 1.0);
}
