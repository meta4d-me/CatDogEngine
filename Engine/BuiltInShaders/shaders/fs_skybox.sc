$input v_worldPos

#include "../common/common.sh"

SAMPLERCUBE(s_texSkybox, 0);

void main()
{
	vec3 dir = normalize(v_worldPos.xyz);
	
	// Use radiance texture mip0.
	vec3 color = toLinear(textureCubeLod(s_texSkybox, dir, 0.0).xyz) * 0.6;
	
	gl_FragColor = vec4(color, 1.0);
}
