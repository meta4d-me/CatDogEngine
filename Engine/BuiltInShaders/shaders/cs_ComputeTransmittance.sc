#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE2D_WR(s_transmittance, rgba32f, 0);

NUM_THREADS(8, 8, 1)
void main()
{
	vec2 uv = gl_GlobalInvocationID.xy;
	ivec2 iuv = ivec2(uv);
	
	vec3 transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(GetAtmosphere(), uv);
	
	imageStore(s_transmittance, iuv, vec4(transmittance.xyz, 1.0));
}
