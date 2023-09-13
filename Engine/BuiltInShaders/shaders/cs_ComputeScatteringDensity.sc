#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "atm_functions.sh"

uniform vec4 u_numScatteringOrders;

IMAGE3D_WR(s_scattering_density, rgba32f, 0);

NUM_THREADS(8, 8, 8)
void main()
{
	ivec3 uvw = ivec3(gl_GlobalInvocationID.xyz);
	int scatteringOrder = u_numScatteringOrders.x;
	
	vec3 density = ComputeScatteringDensityTexture(ATMOSPHERE, uvw, scatteringOrder);
	
	imageStore(s_scattering_density, uvw, vec4(density, 1.0));
}
