#include "../common/bgfx_compute.sh"
#include "../UniformDefines/U_Particle.sh"

BUFFER_RW(AllRibbonParticleVertex, vec4, PT_RIBBON_VERTEX_STAGE);

uniform vec4 u_ribbonCount;
// x : allRibbonVertexCount
// y : particleActiveCount

uniform vec4 u_ribbonMaxPos[300];
//here have max particle count (max limit of inspector)

NUM_THREADS(1u, 1u, 1u)
void main()
{
    for(uint i = 0,index= 0; i <u_ribbonCount.x; i += 2,index++)
    {
        AllRibbonParticleVertex[i] = vec4(
            u_ribbonMaxPos[index].x,
            u_ribbonMaxPos[index].y + 1.0f,
            u_ribbonMaxPos[index].z,
            0.0f
        );

        AllRibbonParticleVertex[i+1] = vec4(
            u_ribbonMaxPos[index].x,
            u_ribbonMaxPos[index].y - 1.0f,
            u_ribbonMaxPos[index].z,
            0.0f
        );
    }
}