#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE2D_WR(s_delta_irradiance, rgba32f, 0);
IMAGE2D_WR(s_irradiance, rgba32f, 1);

NUM_THREADS(8, 8, 1)
void main()
{
	vec2 uv = gl_GlobalInvocationID.xy;
	ivec2 iuv = ivec2(uv);
	
	vec3 delta_irradiance = ComputeDirectIrradianceTexture(GetAtmosphere(), uv);
	vec3 irradiance = vec3_splat(0.0);
	
	imageStore(s_delta_irradiance, iuv, vec4(delta_irradiance.xyz, 1.0));
	imageStore(s_irradiance, iuv, vec4(irradiance.xyz, 1.0));
}
