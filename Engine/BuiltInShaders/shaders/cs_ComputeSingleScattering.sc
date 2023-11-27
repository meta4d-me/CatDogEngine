#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE3D_WR(s_delta_rayleigh_scattering, rgba32f, 0);
IMAGE3D_WR(s_delta_mie_scattering, rgba32f, 1);
IMAGE3D_WR(s_scattering, rgba32f, 2);

NUM_THREADS(8, 8, 8)
void main()
{
	vec3 uvw = gl_GlobalInvocationID.xyz;
	ivec3 iuvw = ivec3(uvw);
	
	vec3 delta_rayleigh = vec3_splat(0.0);
	vec3 delta_mie = vec3_splat(0.0);
	
	ComputeSingleScatteringTexture(GetAtmosphere(), uvw, delta_rayleigh, delta_mie);
	
	imageStore(s_delta_rayleigh_scattering, iuvw, vec4(delta_rayleigh.xyz, 1.0));
	imageStore(s_delta_mie_scattering, iuvw, vec4(delta_mie.xyz, 1.0));
	imageStore(s_scattering, iuvw, vec4(delta_rayleigh.xyz, delta_mie.x));
}
