#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE3D_WR(s_delta_multiple_scattering, rgba32f, 0);
IMAGE3D_RW(s_scattering, rgba32f, 1);

NUM_THREADS(8, 8, 8)
void main()
{
	vec3 uvw = gl_GlobalInvocationID.xyz;
	ivec3 iuvw = ivec3(uvw);
	
	float nu;
	vec3 delta_multiple_scattering = ComputeMultipleScatteringTexture(GetAtmosphere(), uvw, nu);
	vec3 scattering = delta_multiple_scattering / RayleighPhaseFunction(nu);
	
	vec4 lastScattering = imageLoad(s_scattering, iuvw).xyzw;
	
	imageStore(s_delta_multiple_scattering, iuvw, vec4(delta_multiple_scattering, 1.0));
	imageStore(s_scattering, iuvw, lastScattering + vec4(scattering, 0.0));
}
