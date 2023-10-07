#include "../common/bgfx_compute.sh"

BUFFER_RO(allMorphVertexIDIB, uint, 2);
BUFFER_RO(activeMorphOffestLengthIB, uint, 4);
BUFFER_RO(activeMorphWeightVB, vec4, 5);
BUFFER_RW(finalMorphAffectedVB, vec4, 6);

uniform vec4 u_morphCount;
uniform vec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    /*
    uint threadID = gl_GlobalInvocationID.x;
    for(uint i = 0; i*64+threadID <u_vertexCount.x;i++)
    {
        finalMorphAffectedVB[i*64+threadID] =vec4(0,0,0,1.0f);
    }
    if(threadID<u_morphCount.x)
    {
        uint offset = activeMorphOffestLengthIB[threadID*2];
        uint length = activeMorphOffestLengthIB[threadID*2+1];
        float weight = activeMorphWeightVB[threadID].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[offset+j];
            float wgt = finalMorphAffectedVB[id].w - weight;
            finalMorphAffectedVB[id] = vec4(0,0,0,wgt);
        }
    }
    */
    for(uint i = 0; i < u_vertexCount.x; i++)
    {
        finalMorphAffectedVB[i] =vec4(0,0,0,1.0f);
    }
    for(uint i = 0; i <u_morphCount.x;i++)
    {
        uint offset = activeMorphOffestLengthIB[i*2];
        uint length = activeMorphOffestLengthIB[i*2+1];
        float weight = activeMorphWeightVB[i].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[offset+j];
            float finalWeight = finalMorphAffectedVB[id].w - weight;
            finalMorphAffectedVB[id] = vec4(0,0,0,wgt);
        }
    }
    
}