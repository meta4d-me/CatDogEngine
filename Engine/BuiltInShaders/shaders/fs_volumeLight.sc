$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/BRDF.sh"
#include "../common/Material.sh"
#include "../common/Camera.sh"

#include "../common/LightSource.sh"
#include "../common/Envirnoment.sh"

uniform vec4 u_emissiveColor;
uniform vec4 u_mieScatterFactor;
uniform vec4 u_stepLength_density;

uniform vec4 u_scattring_extinction;
//float scatteringFactor = 0.5;
//float extingctionFactor = 0.6;

define mieScatteringFactor  u_mieScatterFactor.xyz;
define ScatteringFactor     u_scattring_extinction.x;
define ExtingctionFactor    u_scattring_extinction.y;

SAMPLER2D(s_shadowMap, SHADOW_MAP_SLOT);

// TODO List
// Multiple lights attributes

vec3 GetLi(float3 wpos)
{

}

float ExtingctionFunc(float stepLength, out float extinction) // extinction will accumulate with ray marching
{
	extinction += ExtingctionFactor * stepLength;
	return exp(-extinction);
}

float MieScatteringFunc(float3 lightDir, float3 rayDir)
{
	// MieScattering using Henyey-Greenstein approximation
	// (1 - g) ^ 2 / (4 * pi * (1 + g ^ 2 - 2 * g * cosθ) ^ 1.5 )
    // mieScatteringFactor.x = (1 - g) ^ 2 / (4 * pi) 
    // mieScatteringFactor.y = 1 + g ^ 2
    // mieScatteringFactor.z = 2 * g
	float lightCos = dot(lightDir, -rayDir);
	return mieScatteringFactor.x / pow((mieScatteringFactor.y - mieScatteringFactor.z * lightCos), 1.5);
}

vec4 RayMarching(vec3 worldCamPos, vec3 rayDir, float rayLength, int stepCount)
{
    float stepLength = rayLength / stepCount;
    float3 stepVec = rayDir * stepLength;
    float3 curWorldPos = worldCamPos;
    
    vec3 totalLo = 0;
    float extinction = 0;//it will accumulate with ray marching

    //Main Body of RayMarching
    for (int i = 0; i < stepCount; ++i)
    {
        //TODO : Medium of Varible Scattering Factors  
        //Now it is assumed that the air is a homogeneous media that has a fixed scattering factor 

        //TODO : Multiple Lights Contribute to the Lo  
        vec3 Lo = vec3(0, 0, 0);
        //for(every light)
        //{
            float3 toLight = worldCamPos - worldSpaceLightPos0.xyz;//
            vec3 Li = GetLi();
            float phase = MieScatteringFunc(normalize(-toLight), rayDir);
            Lo += Li * phase;
        //}
        //Lo with Extingction
        Lo *= ExtingctionFunc(stepLength, extinction);
        Lo *= ScatteringFactor;
        totalLo += Lo;

        //Next Step for Ray Marching
        curWorldPos += stepVec;
    }

    vec4 finalColor = vec3(totalLo, 1.0 - exp(-extinction));
    return finalColor;
}

void main()
{
    vec2 uvCoord = vec2( gl_Position.x / gl_Position.w * 0.5f + 0.5f, gl_Position.xy / gl_Position.w * (-0.5f) + 0.5f);
    float depth = texture(s_shadowMap, uvCoord);

    // reconstruct the wolrd coordinate with shadow map 
    int steps = min(floor(),floor());
    vec3 marchingPos = v_worldPos;
    vec3 stepLength = u_stepLength_density.xyz;
    float marchingDensity = 0;
    float density = u_stepLength_density.w;

    int lightStepssteps;
    vec3 lightMarchingPos;
    vec3 lightStepLength;
    float lightMarchingDepth;
    float lightDepth;

    vec3 finalColor = RayMarching();
}
