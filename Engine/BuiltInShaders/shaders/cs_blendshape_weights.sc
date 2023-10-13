#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(allMorphVertexIDIB,        uint, BS_ALL_MORPH_VERTEX_ID_STAGE);
BUFFER_RO(activeMorphOffestLengthIB, uint, BS_ACTIVE_MORPH_DATA_STAGE);
BUFFER_RW(finalMorphAffectedVB,      vec4, BS_FINAL_MORPH_AFFECTED_STAGE);

uniform vec4 u_morphCount_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0; i <u_morphCount_vertexCount.y;i++)
    {
        finalMorphAffectedVB[i] =vec4(0,0,0,1.0f);
    }
    for(uint i = 0; i <u_morphCount_vertexCount.x;i++)
    {
        uint offset = activeMorphOffestLengthIB[i*3];
        uint length = activeMorphOffestLengthIB[i*3+1];
        float weight = asfloat(activeMorphOffestLengthIB[i*3+2]);
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[(offset+j)*4];
            float wgt = finalMorphAffectedVB[id].w - weight;
            finalMorphAffectedVB[id] = vec4(0,0,0,wgt);
        }
    }
    
}