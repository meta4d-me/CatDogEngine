$input v_worldPos, v_normal, v_texcoord0, v_TBN

#include "../common/common.sh"
#include "../common/DDGI.sh"

SAMPLER2D(s_texBaseColor, 0);

vec3 SampleAlbedo(vec2 uv) {
	return texture2D(s_texBaseColor, uv).xyz;
}

void main()
{
	vec3 albedo = SampleAlbedo(v_texcoord0);
	
	vec3 envDiffuseIrradiance = GetDDGIIrradiance(v_worldPos, v_normal);
	
	gl_FragColor = vec4(albedo * vec3_splat(CD_INV_PI) * envDiffuseIrradiance, 1.0);
}
