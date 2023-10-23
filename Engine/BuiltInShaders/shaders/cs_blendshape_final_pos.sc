#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(allMorphVertexIDIB,        uint, BS_ALL_MORPH_VERTEX_ID_STAGE);
BUFFER_RO(activeMorphOffestLengthIB, uint, BS_ACTIVE_MORPH_DATA_STAGE);
BUFFER_RW(finalMorphAffectedVB,      vec4, BS_FINAL_MORPH_AFFECTED_STAGE);

uniform vec4 u_morphCount_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(int i = 0; i < u_morphCount_vertexCount.x; i++)
    {
        uint offset = activeMorphOffestLengthIB[i*3];
        uint length = activeMorphOffestLengthIB[i*3+1];
        float weight = asfloat(activeMorphOffestLengthIB[i*3+2]);
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[(offset+j)*4];
            float x = asfloat(allMorphVertexIDIB[(offset+j)*4+1]);
            float y = asfloat(allMorphVertexIDIB[(offset+j)*4+2]);
            float z = asfloat(allMorphVertexIDIB[(offset+j)*4+3]);
            finalMorphAffectedVB[id] = vec4(
                finalMorphAffectedVB[id].x +weight*x,
                finalMorphAffectedVB[id].y +weight*y,
                finalMorphAffectedVB[id].z +weight*z,
                finalMorphAffectedVB[id].w
            );
        }
    }
}