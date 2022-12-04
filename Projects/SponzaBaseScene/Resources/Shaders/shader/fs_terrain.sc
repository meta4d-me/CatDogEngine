$input v_worldPos, v_normal, v_texcoord0

#include "../common/common.sh"
#include "uniforms.sh"

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texDebug, 1);

void main()
{
	gl_FragColor = vec4(texture2D(s_texBaseColor, v_texcoord0).xyz, 1.0);
}
