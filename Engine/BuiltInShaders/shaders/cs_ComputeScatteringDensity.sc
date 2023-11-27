#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

uniform vec4 u_numScatteringOrders;

IMAGE3D_WR(s_scattering_density, rgba32f, 0);

NUM_THREADS(8, 8, 8)
void main()
{
	vec3 uvw = gl_GlobalInvocationID.xyz;
	ivec3 iuvw = ivec3(uvw);
	int scatteringOrder = int(u_numScatteringOrders.x);
	
	vec3 density = ComputeScatteringDensityTexture(GetAtmosphere(), uvw, scatteringOrder);
	
	imageStore(s_scattering_density, iuvw, vec4(density, 1.0));
}
