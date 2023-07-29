$input v_worldPos

#include "../common/common.sh"

SAMPLERCUBE(s_texSkybox, 0);

void main()
{
	vec3 dir = normalize(v_worldPos.xyz);
	
	// Use radiance texture mip0.
	// We use the HDR texture which in linear space.
	vec3 color = textureCubeLod(s_texSkybox, dir, 0.0).xyz * vec3_splat(0.6);
	
	gl_FragColor = vec4(color, 1.0);
}
