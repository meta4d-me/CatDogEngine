$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_noiseTexture, 0);
SAMPLER2D(s_positionTexture, 1);
SAMPLER2D(s_normalTexture, 2);

uniform vec4 u_samples[64];
uniform vec4 u_screenSize;

void main()
{
    vec3 position = texture2D(s_positionTexture,v_texcoord0).xyz;
    vec3 normal = normalize(texture2D(s_normalTexture,v_texcoord0).xyz); 
    vec2 noiseScale = vec2(u_screenSize.x / 4.0, u_screenSize.y / 4.0);
    vec3 randomVec = normalize(texture2D(s_noiseTexture, v_texcoord0 * noiseScale).xyz);

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = normalize(cross(normal, tangent));
    mat3 TBN = mtxFromCols(tangent, bitangent, normal);    

    int kernelSize = 64;
    float radius = 1;

    float occlusion = 0.0;
    for(int i = 0; i < kernelSize; ++i)
    {
        vec3 sample = mul(u_samples[i].xyz,TBN);
        sample = position + sample * radius; 
        
        vec4 offset = vec4(sample, 1.0);
        offset = mul(u_proj , offset); 
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5; 
        
        float sampleDepth = -texture2D(s_positionTexture, offset.xy).w;
        
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(position.z - sampleDepth ));
        occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;           
    }
    occlusion = 1.0 - (occlusion / kernelSize);    

    gl_FragColor = vec4(occlusion,occlusion,occlusion,1.0);
}