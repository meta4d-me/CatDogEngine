$input v_worldPos

#include "../common/common.sh"
#include "atm_functions.sh"

uniform vec4 u_cameraPos[1];
uniform vec4 u_LightDir[1];

void main()
{
	vec3 rayStart = u_cameraPos[0].xyz + vec3(0.0, ATMOSPHERE.bottom_radius , 0.0);
	vec3 rayDir = normalize(v_worldPos.xyz);
	vec3 sunDir = -normalize(u_LightDir[0].xyz);
	
	vec3 trans;
	// TODO : Need a shadow volume algorithm to get shadow_length parameter to compute god ray.
	vec3 scattering = GetSkyRadiance(ATMOSPHERE, rayStart, rayDir, 0.0, sunDir, trans);
	
	gl_FragColor = vec4(scattering, 1.0);
}
