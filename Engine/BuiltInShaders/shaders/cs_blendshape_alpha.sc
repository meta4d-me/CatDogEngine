#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(morphAffectedVB,           vec4, BS_MORPH_AFFECTED_STAGE);
BUFFER_RO(finalMorphAffectedVB,      vec4, BS_FINAL_MORPH_AFFECTED_STAGE);
BUFFER_RW(alphaVertex,               vec4, BS_ALPHA_STAGE);

uniform vec4 u_morphCount_vertexCount;
uniform vec4 u_overallWeight;

#define alpha u_overallWeight.x

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(int i = 0; i < u_morphCount_vertexCount.y; i++)
    {
        alphaVertex[i] = vec4((1-alpha)*morphAffectedVB[i].x + alpha*finalMorphAffectedVB[i].x,
                                (1-alpha)*morphAffectedVB[i].y + alpha*finalMorphAffectedVB[i].y,
                                (1-alpha)*morphAffectedVB[i].z + alpha*finalMorphAffectedVB[i].z,
                                1.0);
    }
}