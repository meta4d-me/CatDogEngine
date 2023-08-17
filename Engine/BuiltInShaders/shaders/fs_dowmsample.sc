$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_tex, 0);
uniform vec4 s_tex_size;

void main()
{
    vec2 halfpixel = 0.5 * vec2(s_tex_size.x, s_tex_size.y);
    vec2 oneepixel = 1.0 * vec2(s_tex_size.x, s_tex_size.y);

    vec2 uv = v_texcoord0;

    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);

    sum += (4.0/32.0) * texture2D(s_tex, uv);

    sum += (4.0/32.0) * texture2D(s_tex, uv + vec2(-halfpixel.x, -halfpixel.y) );
    sum += (4.0/32.0) * texture2D(s_tex, uv + vec2(+halfpixel.x, +halfpixel.y) );
    sum += (4.0/32.0) * texture2D(s_tex, uv + vec2(+halfpixel.x, -halfpixel.y) );
    sum += (4.0/32.0) * texture2D(s_tex, uv + vec2(-halfpixel.x, +halfpixel.y) );

    sum += (2.0/32.0) * texture2D(s_tex, uv + vec2(+oneepixel.x, 0.0) );
    sum += (2.0/32.0) * texture2D(s_tex, uv + vec2(-oneepixel.x, 0.0) );
    sum += (2.0/32.0) * texture2D(s_tex, uv + vec2(0.0, +oneepixel.y) );
    sum += (2.0/32.0) * texture2D(s_tex, uv + vec2(0.0, -oneepixel.y) );
    sum += (1.0/32.0) * texture2D(s_tex, uv + vec2(+oneepixel.x, +oneepixel.y) );
    sum += (1.0/32.0) * texture2D(s_tex, uv + vec2(-oneepixel.x, +oneepixel.y) );
    sum += (1.0/32.0) * texture2D(s_tex, uv + vec2(+oneepixel.x, -oneepixel.y) );
    sum += (1.0/32.0) * texture2D(s_tex, uv + vec2(-oneepixel.x, -oneepixel.y) );   

    gl_FragColor = sum;
}