#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(morphAffectedVB,           vec4, BS_MORPH_AFFECTED_STAGE);
BUFFER_RO(allMorphVertexIDIB,        uint, BS_ALL_MORPH_VERTEX_ID_STAGE);
BUFFER_RO(activeMorphOffestLengthIB, uint, BS_ACTIVE_MORPH_DATA_STAGE);
BUFFER_RW(finalMorphAffectedVB,      vec4, BS_FINAL_MORPH_AFFECTED_STAGE);
BUFFER_RO(changedIndex,              uint, BS_CHANGED_MORPH_INDEX_STAGE);

uniform vec4 u_changedWeight;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    uint changedId = changedIndex[0];
    uint offset = activeMorphOffestLengthIB[0*3];
    uint length = activeMorphOffestLengthIB[0*3+1];
    float weight = asfloat(activeMorphOffestLengthIB[0*3+2]) - u_changedWeight.x;
    for(uint j = 0; j < length; j++)
    {
        uint id = allMorphVertexIDIB[(offset+j)*4];
        float x = asfloat(allMorphVertexIDIB[(offset+j)*4+1]);
        float y = asfloat(allMorphVertexIDIB[(offset+j)*4+2]);
        float z = asfloat(allMorphVertexIDIB[(offset+j)*4+3]);
        finalMorphAffectedVB[id] = vec4(
            finalMorphAffectedVB[id].x + weight*(x-morphAffectedVB[id].x),
            finalMorphAffectedVB[id].y + weight*(y-morphAffectedVB[id].y),
            finalMorphAffectedVB[id].z + weight*(z-morphAffectedVB[id].z),
            finalMorphAffectedVB[id].w
        );
    } 
}