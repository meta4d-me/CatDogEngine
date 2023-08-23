#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "atm_functions.sh"

IMAGE3D_WR(s_delta_rayleigh_scattering, rgba32f, 0);
IMAGE3D_WR(s_delta_mie_scattering, rgba32f, 1);
IMAGE3D_WR(s_scattering, rgba32f, 2);

NUM_THREADS(8, 8, 8)
void main()
{
	ivec3 uvw = ivec3(gl_GlobalInvocationID.xyz);
	
	vec3 delta_rayleigh = vec3_splat(0.0);
	vec3 delta_mie = vec3_splat(0.0);
	
	ComputeSingleScatteringTexture(ATMOSPHERE, uvw, delta_rayleigh, delta_mie);
	
	imageStore(s_delta_rayleigh_scattering, uvw, vec4(delta_rayleigh.xyz, 1.0));
	imageStore(s_delta_mie_scattering, uvw, vec4(delta_mie.xyz, 1.0));
	imageStore(s_scattering, uvw, vec4(delta_rayleigh.xyz, delta_mie.x));
}
