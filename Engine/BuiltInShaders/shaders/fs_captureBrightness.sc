$input v_texcoord0

#include "../common/common.sh"

SAMPLER2D(s_texture, 0);

uniform vec4 u_luminanceThreshold;

float luminance(vec4 color){
    return 0.2126729f  * color.r + 0.7151522f  * color.g + 0.0721750f * color.b;
}

void main()
{
    vec4 col = texture2D(s_texture, v_texcoord0);
    float intensity = saturate(luminance(col) - u_luminanceThreshold.x);
    gl_FragColor = vec4(col.rgb * intensity,1.0);
}