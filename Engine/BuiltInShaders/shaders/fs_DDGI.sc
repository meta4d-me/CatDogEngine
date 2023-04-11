$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "uniforms.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

SAMPLER2D(s_texBaseColor, 0);

vec3 SampleAlbedoTexture(vec2 uv) {
	return texture2D(s_texBaseColor, uv).xyz;
}

void main()
{
	gl_FragColor = vec4(SampleAlbedoTexture(v_texcoord0), 1.0);
}
