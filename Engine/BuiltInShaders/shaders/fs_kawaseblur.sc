$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_texture, 0);
uniform vec4 u_textureSize;

void main()
{
    vec4 o = vec4(0.0, 0.0, 0.0, 0.0);
    o += texture2D(s_texture, v_texcoord0 + vec2(+u_textureSize.z + 0.5, +u_textureSize.z + 0.5) * u_textureSize.xy); 
    o += texture2D(s_texture, v_texcoord0 + vec2(-u_textureSize.z - 0.5, +u_textureSize.z + 0.5) * u_textureSize.xy); 
    o += texture2D(s_texture, v_texcoord0 + vec2(-u_textureSize.z - 0.5, -u_textureSize.z - 0.5) * u_textureSize.xy); 
    o += texture2D(s_texture, v_texcoord0 + vec2(+u_textureSize.z + 0.5, -u_textureSize.z - 0.5) * u_textureSize.xy); 
    o *= 0.25;
    gl_FragColor = o;
}