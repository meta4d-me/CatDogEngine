#include "../common/bgfx_compute.sh"

BUFFER_RW(dynamicPosBuffer, vec4, 6);
BUFFER_RO(staticBuffer1, uint, 2);
BUFFER_RO(staticBuffer2, vec4, 3);
BUFFER_RO(dynamicBuffer1, uint, 4);
BUFFER_RO(dynamicBuffer2, vec4, 5);

uniform vec4 u_morphCount;
uniform vec4 u_vertexCount;

NUM_THREADS(64u, 1u, 1u)
void main()
{
    for(int i = 0; i <u_morphCount.x;i++)
    {
        uint offset = dynamicBuffer1[i*2];
        uint length = dynamicBuffer1[i*2+1];
        float weight = dynamicBuffer2[i].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = staticBuffer1[offset+j];
            float x = staticBuffer2[offset+j].x;
            float y = staticBuffer2[offset+j].y;
            float z = staticBuffer2[offset+j].z;
            dynamicPosBuffer[id] = vec4(
                dynamicPosBuffer[id].x +weight*x,
                dynamicPosBuffer[id].y +weight*y,
                dynamicPosBuffer[id].z +weight*z,
                dynamicPosBuffer[id].w
            );
        }
    }
}