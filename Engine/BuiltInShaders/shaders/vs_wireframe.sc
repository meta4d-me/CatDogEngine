$input a_position, a_color0
$output v_bc

#include "../common/common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_bc = vec3(a_color0.x, a_color0.y, a_color0.z);
}