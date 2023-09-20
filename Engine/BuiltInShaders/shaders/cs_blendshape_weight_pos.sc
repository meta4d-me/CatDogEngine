#include "../common/bgfx_compute.sh"

BUFFER_RO(staticPosBuffer, vec4, 1);
BUFFER_RW(dynamicPosWeightBuffer, vec4, 6);

uniform vec4 u_vertexCount;

NUM_THREADS(64u, 1u, 1u)
void main()
{
    for(uint i = 0; i <u_vertexCount.x;i++)
    {
        dynamicPosWeightBuffer[i]=vec4(
            dynamicPosWeightBuffer[i].w*staticPosBuffer[i].x,
            dynamicPosWeightBuffer[i].w*staticPosBuffer[i].y,
            dynamicPosWeightBuffer[i].w*staticPosBuffer[i].z,
            dynamicPosWeightBuffer[i].w
        );
    }
}