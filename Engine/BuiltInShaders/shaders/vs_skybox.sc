$input a_position
$output v_worldPos

#include "../common/common.sh"

void main()
{
	v_worldPos = a_position;
	
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
}
