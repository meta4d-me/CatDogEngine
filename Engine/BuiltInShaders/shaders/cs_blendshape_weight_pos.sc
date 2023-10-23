#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(morphAffectedVB,      vec4, BS_MORPH_AFFECTED_STAGE);
BUFFER_RW(finalMorphAffectedVB, vec4, BS_FINAL_MORPH_AFFECTED_STAGE);

uniform vec4 u_morphCount_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0; i < u_morphCount_vertexCount.y; i++)
    {
        finalMorphAffectedVB[i]=vec4(
            finalMorphAffectedVB[i].w*morphAffectedVB[i].x,
            finalMorphAffectedVB[i].w*morphAffectedVB[i].y,
            finalMorphAffectedVB[i].w*morphAffectedVB[i].z,
            finalMorphAffectedVB[i].w
        );
    }
}