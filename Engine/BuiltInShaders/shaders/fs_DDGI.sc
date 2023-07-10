$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"

#define PI 3.1415926536
#define PI2 9.8696044011
#define INV_PI 0.3183098862
#define INV_PI2 0.1013211836

SAMPLER2D(s_texBaseColor, 0);
SAMPLER2D(s_texClassification, 1);
SAMPLER2D(s_texDistance, 2);
SAMPLER2D(s_texIrradiance, 3);
SAMPLER2D(s_texRelocation, 4);

vec3 SampleAlbedoTexture(vec2 uv) {
	return texture2D(s_texDistance, uv).xyz;
	// return vec3_splat(5.0) * texture2D(s_texIrradiance, uv).xyz;
}

void main()
{
	gl_FragColor = vec4(SampleAlbedoTexture(v_texcoord0), 1.0);
}
