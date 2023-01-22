$input a_position, a_bitangent
$output v_bc

#include "../common/common.sh"
#include "uniforms.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_bc = a_bitangent;
}