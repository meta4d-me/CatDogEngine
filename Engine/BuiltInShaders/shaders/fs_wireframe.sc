$input v_bc

#include "../common/common.sh"
#include "uniforms.sh"

void main()
{
	vec3  color     = vec3(1.0, 0.02, 0.01);
	float opacity   = 1.0;
	float thickness = 1.0;
	vec3 fw = abs(dFdx(v_bc)) + abs(dFdy(v_bc));
	vec3 val = smoothstep(vec3_splat(0.0), fw * thickness, v_bc);
	
	// Gets to 0.0 around the edges.
	float edge = min(min(val.x, val.y), val.z);
	
	if (gl_FrontFacing) { opacity = 0.5; }
	vec4 wfColor = vec4(color, (1.0 - edge) * opacity);
	
	gl_FragColor = wfColor;
}
