#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "atm_functions.sh"

uniform vec4 u_num_scattering_orders[1];

IMAGE3D_WR(s_scattering_density, rgba32f, 8);

NUM_THREADS(8, 8, 8)
void main()
{
	ivec3 uvw = ivec3(gl_GlobalInvocationID.xyz);
	int scatteringOrder = u_num_scattering_orders[0].x;
	
	vec3 density = ComputeScatteringDensityTexture(ATMOSPHERE, uvw, scatteringOrder);
	
	imageStore(s_scattering_density, uvw, vec4(density, 1.0));
}
