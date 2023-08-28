$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_texture, 0);
uniform vec4 u_textureSize;
uniform vec4 u_bloomIntensity;

void main()
{
    vec4 sum = vec4(0.0, 0.0, 0.0, 0.0);
    sum += (4.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy);

    sum += (2.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2(-u_textureSize.x,  0.0) );
    sum += (2.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2( 0.0,          u_textureSize.y) );
    sum += (2.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2( u_textureSize.x,  0.0) );
    sum += (2.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2( 0.0,         -u_textureSize.y) );

    sum += (1.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2(-u_textureSize.x, -u_textureSize.y) );
    sum += (1.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2(-u_textureSize.x,  u_textureSize.y) );
    sum += (1.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2( u_textureSize.x, -u_textureSize.y) );
    sum += (1.0 / 16.0) * texture2D(s_texture, v_texcoord0.xy + vec2( u_textureSize.x,  u_textureSize.y) );

    gl_FragColor = sum * u_bloomIntensity.x;
}