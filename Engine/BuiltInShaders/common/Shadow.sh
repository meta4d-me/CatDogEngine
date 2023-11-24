#include "../UniformDefines/U_Shadow.sh"

uniform mat4 u_lightViewProj;//[LIGHT_NUM]
uniform float u_bias;//[LIGHT_NUM]

SAMPLER2D(s_texShadowMap, SHADOW_MAP_SLOT);

float CalculateShadow(vec3 fragPosWorldSpace, vec3 normal, vec3 lightDir)//add parameter : light index?
{
    float shadow = 0.0;
    return shadow;
/*
    // viewproj coordinate to light space
    vec4 fragPosLightSpace = mul(u_lightViewProj, vec4(fragPosWorldSpace, 1.0));
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture2DLod(s_texShadowMap, projCoords.xy, 0).r; 
    // Calculate bias (based on depth map resolution and slope)
    float bias = 0.0;
    //max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // Check whether current frag pos is in shadow
    float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    // PCF
    //float shadow = 0.0;
    //vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    //for(int x = -1; x <= 1; ++x)
    //{
    //    for(int y = -1; y <= 1; ++y)
    //    {
    //        float pcfDepth = texture2D(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
    //        shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    //    }    
    //}
    //shadow /= 9.0;

    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
*/
}


