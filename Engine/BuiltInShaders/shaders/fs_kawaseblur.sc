$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_tex, 0);
uniform vec4 s_tex_size;

void main()
{
    vec4 o = vec4(0.0,0.0,0.0,0.0);
    o += texture2D(s_tex, v_texcoord0 + vec2(s_tex_size.z +0.5, s_tex_size.z +0.5) * s_tex_size.xy); 
    o += texture2D(s_tex, v_texcoord0 + vec2(-s_tex_size.z -0.5, s_tex_size.z +0.5) * s_tex_size.xy); 
    o += texture2D(s_tex, v_texcoord0 + vec2(-s_tex_size.z -0.5, -s_tex_size.z -0.5) * s_tex_size.xy); 
    o += texture2D(s_tex, v_texcoord0 + vec2(s_tex_size.z +0.5, -s_tex_size.z -0.5) * s_tex_size.xy); 
    o *= 0.25;
    gl_FragColor = o;
}