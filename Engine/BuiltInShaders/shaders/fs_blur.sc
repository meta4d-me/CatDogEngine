$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_tex, 0);
uniform vec4 s_tex_size;

void main()
{
    vec2 tex_offset = s_tex_size.xy;
    vec2 uv = v_texcoord0.xy;
    bool horizontal = s_tex_size.z == 1.0 ? true : false;
    float blurSize = s_tex_size.w;

    float weight[5] = {0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216};

    vec3 result = texture2D(s_tex, uv).rgb * weight[0]; 
    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture2D(s_tex, uv + vec2(tex_offset.x * i, 0.0) * blurSize).rgb * weight[i];
            result += texture2D(s_tex, uv - vec2(tex_offset.x * i, 0.0) * blurSize).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture2D(s_tex, uv + vec2(0.0,tex_offset.y * i) * blurSize).rgb * weight[i];
            result += texture2D(s_tex, uv - vec2(0.0,tex_offset.y * i) * blurSize).rgb * weight[i];
        }
    }

    gl_FragColor = vec4(result, 1.0);
}