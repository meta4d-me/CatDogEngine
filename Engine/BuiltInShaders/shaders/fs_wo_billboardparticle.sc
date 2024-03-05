$input v_color0, v_texcoord0
#include "../common/common.sh"

SAMPLER2D(s_texColor, 0);

void main()
{
	vec4 rgba = texture2D(s_texColor, v_texcoord0.xy);

	rgba.xyz = rgba.xyz * v_color0.xyz;
	rgba.w   = rgba.w * v_color0.w;
	gl_FragColor = rgba;
}