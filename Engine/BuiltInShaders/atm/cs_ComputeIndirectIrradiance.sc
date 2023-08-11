#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "atm_functions.sh"

uniform vec4 u_numScatteringOrders;

IMAGE2D_WR(s_delta_irradiance, rgba32f, 8);
IMAGE2D_WR(s_irradiance, rgba32f, 9);

NUM_THREADS(8, 8, 1)
void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	int scatteringOrder = u_numScatteringOrders.x;
	
	vec3 deltaIrradiance = ComputeIndirectIrradianceTexture(ATMOSPHERE, uv, scatteringOrder);
	
	vec3 lastIrradiance = imageLoad(s_irradiance, uv);
	
	imageStore(s_delta_irradiance, uv, vec4(deltaIrradiance, 1.0));
	imageStore(s_irradiance, uv, vec4(lastIrradiance + deltaIrradiance, 1.0));
}
