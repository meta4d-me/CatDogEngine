#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_BlendShape.sh"

BUFFER_RO(allMorphVertexIDIB, uint, 2);
BUFFER_RO(allMorphVertexPosVB, vec4, 3);
BUFFER_RO(activeMorphOffestLengthIB, uint, 4);
BUFFER_RO(activeMorphWeightVB, vec4, 5);
BUFFER_RW(finalMorphAffectedVB, vec4, 6);

uniform vec4 u_morphCount;
uniform vec4 u_vertexCount;

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(int i = 0; i <u_morphCount.x;i++)
    {
        uint offset = activeMorphOffestLengthIB[i*2];
        uint length = activeMorphOffestLengthIB[i*2+1];
        float weight = activeMorphWeightVB[i].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[offset+j];
            float x = allMorphVertexPosVB[offset+j].x;
            float y = allMorphVertexPosVB[offset+j].y;
            float z = allMorphVertexPosVB[offset+j].z;
            finalMorphAffectedVB[id] = vec4(
                finalMorphAffectedVB[id].x +weight*x,
                finalMorphAffectedVB[id].y +weight*y,
                finalMorphAffectedVB[id].z +weight*z,
                finalMorphAffectedVB[id].w
            );
        }
    }
    /*
    uint threadId = gl_GlobalInvocationID.x;
    if(threadId < u_morphCount.x){
        uint offset = activeMorphOffestLengthIB[threadId*2];
        uint length = activeMorphOffestLengthIB[threadId*2+1];
        float weight = activeMorphWeightVB[threadId].x;
        for(uint j = 0; j < length; j++)
        {
            uint id = allMorphVertexIDIB[offset+j];
            float x = allMorphVertexPosVB[offset+j].x;
            float y = allMorphVertexPosVB[offset+j].y;
            float z = allMorphVertexPosVB[offset+j].z;
            vec4 fma= finalMorphAffectedVB[id];
            finalMorphAffectedVB[id] = vec4(
                fma.x +weight*x,
                fma.y +weight*y,
                fma.z +weight*z,
                fma.w
            );
        }   
    }
    */
     
}