#include "../common/bgfx_compute.sh"

BUFFER_RO(morphAffectedVB, vec4, 1);
BUFFER_RW(finalMorphAffectedVB, vec4, 6);

uniform vec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0; i <u_vertexCount.x;i++)
    {
        finalMorphAffectedVB[i]=vec4(
            finalMorphAffectedVB[i].w*morphAffectedVB[i].x,
            finalMorphAffectedVB[i].w*morphAffectedVB[i].y,
            finalMorphAffectedVB[i].w*morphAffectedVB[i].z,
            finalMorphAffectedVB[i].w
        );
    }
}