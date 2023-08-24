$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_texture, 0);
uniform vec4 u_textureSize;

void main()
{
    float weight[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};
    vec3 result = texture2D(s_texture, v_texcoord0.xy).rgb * weight[0]; 
    for(int i = 1; i < 5; ++i)
    {
        result += texture2D(s_texture, v_texcoord0.xy + vec2(u_textureSize.x * i, 0.0) * u_textureSize.z).rgb * weight[i];
        result += texture2D(s_texture, v_texcoord0.xy - vec2(u_textureSize.x * i, 0.0) * u_textureSize.z).rgb * weight[i];
    }

    gl_FragColor = vec4(result, 1.0);
}