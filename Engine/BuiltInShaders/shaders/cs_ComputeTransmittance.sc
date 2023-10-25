#include "../common/bgfx_compute.sh"

#define COMPUTE
#include "../common/atm_functions.sh"

IMAGE2D_WR(s_transmittance, rgba32f, 0);

NUM_THREADS(8, 8, 1)
void main()
{
	ivec2 uv = ivec2(gl_GlobalInvocationID.xy);
	
	vec3 transmittance = ComputeTransmittanceToTopAtmosphereBoundaryTexture(ATMOSPHERE, uv);
	
	imageStore(s_transmittance, uv, vec4(transmittance.xyz, 1.0));
}
