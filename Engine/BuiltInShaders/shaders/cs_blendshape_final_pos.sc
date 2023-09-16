#include "../common/bgfx_compute.sh"

BUFFER_RW(dynamicPosBuffer, float, 1);
BUFFER_RO(staticBuffer1, uint, 2);
BUFFER_RO(staticBuffer2, float, 3);
BUFFER_RO(dynamicBuffer1, uint, 4);
BUFFER_RO(dynamicBuffer2, float, 5);

uniform uvec4 u_morphCount;
uniform uvec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0; i <u_morphCount.x;i++)
    {
        uint offset = dynamicBuffer1[i*2];
        uint length = dynamicBuffer1[i*2+1];
        float weight = dynamicBuffer2[i];
        for(uint j = 0; j < length; j++)
        {
            uint id = staticBuffer1[offset+j];
            float x = staticBuffer2[(offset+j)*3];
            float y = staticBuffer1[(offset+j)*3+1];
            float z = staticBuffer1[(offset+j)*3+2];
            dynamicPosBuffer[id] +=weight*x;
            dynamicPosBuffer[id+1] +=weight*y;
            dynamicPosBuffer[id+2] +=weight*z;
        }
    }
}