$input v_texcoord0

#include "../common/common.sh"
#include "../common/ToneMapping.sh"

SAMPLER2D(s_lightingColor, 0);
uniform vec4 u_postProcessingParams;
#define u_exposure float(u_postProcessingParams.x)
#define u_toneMappingMode int(u_postProcessingParams.y)
#define u_gamma float(u_postProcessingParams.z)

// Exposure
//float ComputeEV100(float aperture ,float shutterTime ,float ISO) {
//	// EV number is defined as:
//	// 2^ EV_s = N^2 / t and EV_s = EV_100 + log2 (S /100)
//	// This gives
//	// EV_s = log2 (N^2 / t)
//	// EV_100 + log2 (S /100) = log2 (N^2 / t)
//	// EV_100 = log2 (N^2 / t) - log2 (S /100)
//	// EV_100 = log2 (N^2 / t . 100 / S)
//	
//	return log2(aperture * aperture / shutterTime * 100.0 / ISO);
//}

//float ConvertEV100ToExposure(float EV100) {
//	// Compute the maximum luminance possible with H_sbs sensitivity
//	// maxLum = 78 / ( S * q ) * N^2 / t
//	// = 78 / ( S * q ) * 2^ EV_100
//	// = 78 / (100 * 0.65) * 2^ EV_100
//	// = 1.2 * 2^ EV
//	// Reference : http :// en. wikipedia . org / wiki / Film_speed
//	
//	return 1.0 / (1.2 * exp2(EV100));
//}

void main()
{
	vec3 color = texture2D(s_lightingColor, v_texcoord0).rgb;
	
	// Exposure
	color *= u_exposure;
	
	// Tone Mapping
	color = ToneMapping(color, u_toneMappingMode);
	
	// Gamma Correction
	color = pow(color, vec3_splat(u_gamma));
	
	gl_FragColor = vec4(color, 1.0);
}
