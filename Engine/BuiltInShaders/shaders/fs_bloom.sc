$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_lightingColor, 0);
SAMPLER2D(s_bloom, 1);

void main()
{
    vec3 light = texture2D(s_lightingColor,v_texcoord0).rgb;
    vec3 bloom = texture2D(s_bloom,v_texcoord0).rgb;

    gl_FragColor = vec4(light + bloom,1.0);
}