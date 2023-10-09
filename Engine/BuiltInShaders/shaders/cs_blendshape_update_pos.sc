#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(morphAffectedVB, vec4, 1);
BUFFER_RO(allMorphVertexIDIB, uint, 2);
BUFFER_RO(allMorphVertexPosVB, vec4, 3);
BUFFER_RO(activeMorphOffestLengthIB, uint, 4);
BUFFER_RO(activeMorphWeightVB, vec4, 5);
BUFFER_RW(finalMorphAffectedVB, vec4, 6);
BUFFER_RO(changedIndex, uint, 7);

uniform vec4 u_changedWeight;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    uint changedId = changedIndex[0];
    uint offset = activeMorphOffestLengthIB[0*2];
    uint length = activeMorphOffestLengthIB[0*2+1];
    float weight = activeMorphWeightVB[0].x - u_changedWeight.x;
    for(uint j = 0; j < length; j++)
    {
        uint id = allMorphVertexIDIB[offset+j];
        float x = allMorphVertexPosVB[offset+j].x;
        float y = allMorphVertexPosVB[offset+j].y;
        float z = allMorphVertexPosVB[offset+j].z;
        finalMorphAffectedVB[id] = vec4(
            finalMorphAffectedVB[id].x + weight*(x-morphAffectedVB[id].x),
            finalMorphAffectedVB[id].y + weight*(y-morphAffectedVB[id].y),
            finalMorphAffectedVB[id].z + weight*(z-morphAffectedVB[id].z),
            finalMorphAffectedVB[id].w
        );
    } 
}