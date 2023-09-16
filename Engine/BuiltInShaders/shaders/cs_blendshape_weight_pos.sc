#include "../common/bgfx_compute.sh"

BUFFER_RW(dynamicPosBuffer, float, 1);
BUFFER_RO(sourceWeightBuffer, float, 6);

uniform uvec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0; i <u_vertexCount.x;i++)
    {
        dynamicPosBuffer[i*3]*=sourceWeightBuffer[i];
        dynamicPosBuffer[i*3+1]*=sourceWeightBuffer[i];
        dynamicPosBuffer[i*3+2]*=sourceWeightBuffer[i];
    }
}