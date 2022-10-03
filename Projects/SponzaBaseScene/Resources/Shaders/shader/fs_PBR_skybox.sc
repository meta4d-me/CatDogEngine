$input v_skyboxDir

#include "../common/common.sh"
#include "uniforms.sh"

SAMPLERCUBE(s_texCube, 0);

void main()
{
	vec3 dir = normalize(v_skyboxDir);

	dir = fixCubeLookup(dir, 0.0, 256.0);
	// Use mip0 as texture
	vec3 color = toLinear(textureCubeLod(s_texCube, dir, 0.0).xyz) * 0.3;

	gl_FragColor = vec4(color, 1.0);
}
