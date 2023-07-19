$input v_worldPos

#include "../common/common.sh"
#include "../common/Camera.sh"

SAMPLERCUBE(s_texSkybox, 0);

void main()
{
	vec3 cameraPos = GetCamera().position;
	vec3 dir = normalize(v_worldPos - cameraPos);
	
	// Use radiance texture mip0.
	vec3 color = toLinear(textureCubeLod(s_texSkybox, dir, 0.0).xyz) * 0.5;
	color = vec3(1.0, 0.0, 0.0);
	gl_FragColor = vec4(color, 1.0);
}
