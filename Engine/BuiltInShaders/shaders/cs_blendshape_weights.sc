#include "../common/bgfx_compute.sh"

BUFFER_RO(staticBuffer1, uint, 2);
BUFFER_RO(dynamicBuffer1, uint, 4);
BUFFER_RO(dynamicBuffer2, vec4, 5);
BUFFER_RW(dynamicPosWeightBuffer, vec4, 6);

uniform vec4 u_morphCount;
uniform vec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(int i = 0; i <u_vertexCount.x;i++)
    {
        dynamicPosWeightBuffer[i] =vec4(0,0,0,1.0f);
    }
    for(int i = 0; i <u_morphCount.x;i++)
    {
        uint offset = dynamicBuffer1[i*2];
        uint length = dynamicBuffer1[i*2+1];
        float weight = dynamicBuffer2[i].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = staticBuffer1[offset+j];
            float wgt = dynamicPosWeightBuffer[id].w - weight;
            dynamicPosWeightBuffer[id] = vec4(0,0,0,wgt);
        }
    }
}