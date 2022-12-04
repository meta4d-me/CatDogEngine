$input a_position, a_normal, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0

#include "../common/common.sh"
#include "uniforms.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal     = normalize(mul(u_modelInvTrans, vec4(a_normal, 0.0)).xyz);
	v_texcoord0 = a_texcoord0;
}
