#include "../common/bgfx_compute.sh"

BUFFER_RO(staticBuffer1, uint, 2);
BUFFER_RO(dynamicBuffer1, uint, 4);
BUFFER_RO(dynamicBuffer2, float, 5);
BUFFER_RW(sourceWeightBuffer, float, 6);

uniform uvec4 u_morphCount;
uniform uvec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    sourceWeightBuffer[0] = 1.0f;
    for(uint i = 0; i <u_vertexCount.x;i++)
    {
        sourceWeightBuffer[i] = 1.0f;
    }
    for(uint i = 0; i <u_morphCount.x;i++)
    {
        uint offset = dynamicBuffer1[i*2];
        uint length = dynamicBuffer1[i*2+1];
        float weight = dynamicBuffer2[i];
        for(uint j = 0; j < length; j++)
        {
            uint id = staticBuffer1[offset+j];
            sourceWeightBuffer[id] -=weight;
        }
    }
}