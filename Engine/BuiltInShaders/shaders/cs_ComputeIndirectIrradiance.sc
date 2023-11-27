#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

uniform vec4 u_numScatteringOrders;

IMAGE2D_WR(s_delta_irradiance, rgba32f, 0);
IMAGE2D_RW(s_irradiance, rgba32f, 1);

NUM_THREADS(8, 8, 1)
void main()
{
	vec2 uv = gl_GlobalInvocationID.xy;
	ivec2 iuv = ivec2(uv);
	int scatteringOrder = int(u_numScatteringOrders.x);
	
	vec3 deltaIrradiance = ComputeIndirectIrradianceTexture(GetAtmosphere(), uv, scatteringOrder);
	
	vec3 lastIrradiance = imageLoad(s_irradiance, iuv).xyz;
	
	imageStore(s_delta_irradiance, iuv, vec4(deltaIrradiance, 1.0));
	imageStore(s_irradiance, iuv, vec4(lastIrradiance + deltaIrradiance, 1.0));
}
