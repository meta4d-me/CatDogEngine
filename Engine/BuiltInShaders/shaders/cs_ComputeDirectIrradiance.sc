#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE2D_WR(s_delta_irradiance, rgba32f, 0);
IMAGE2D_WR(s_irradiance, rgba32f, 1);

NUM_THREADS(8, 8, 1)
void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	
	vec3 delta_irradiance = ComputeDirectIrradianceTexture(ATMOSPHERE, uv);
	vec3 irradiance = vec3_splat(0.0);
	
	imageStore(s_delta_irradiance, uv, vec4(delta_irradiance.xyz, 1.0));
	imageStore(s_irradiance, uv, vec4(irradiance.xyz, 1.0));
}
