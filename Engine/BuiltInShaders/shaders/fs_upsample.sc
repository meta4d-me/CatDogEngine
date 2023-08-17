$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_tex, 0);
uniform vec4 s_tex_size;
uniform vec4 _bloomIntensity;

void main()
{
    vec2 halfpixel = s_tex_size.xy;
    vec2 uv = v_texcoord0;

    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

    sum += (4.0 / 16.0) * texture2D(s_tex, uv);

    sum += (2.0 / 16.0) * texture2D(s_tex, uv + vec2(-halfpixel.x,  0.0) );
    sum += (2.0 / 16.0) * texture2D(s_tex, uv + vec2( 0.0,          halfpixel.y) );
    sum += (2.0 / 16.0) * texture2D(s_tex, uv + vec2( halfpixel.x,  0.0) );
    sum += (2.0 / 16.0) * texture2D(s_tex, uv + vec2( 0.0,         -halfpixel.y) );

    sum += (1.0 / 16.0) * texture2D(s_tex, uv + vec2(-halfpixel.x, -halfpixel.y) );
    sum += (1.0 / 16.0) * texture2D(s_tex, uv + vec2(-halfpixel.x,  halfpixel.y) );
    sum += (1.0 / 16.0) * texture2D(s_tex, uv + vec2( halfpixel.x, -halfpixel.y) );
    sum += (1.0 / 16.0) * texture2D(s_tex, uv + vec2( halfpixel.x,  halfpixel.y) );

    gl_FragColor = sum * _bloomIntensity.x;
}