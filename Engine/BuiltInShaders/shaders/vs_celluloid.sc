$input a_position, a_normal, a_color0
$output v_worldPos, v_normal, v_bc

#include "../common/common.sh"

void main()
{
	float outline = 0.05;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal = normalize(mul(u_modelInvTrans, vec4(a_normal, 0.0)).xyz);

	vec3 normal = v_normal;
	normal = normalize(mul(u_view,vec4(normal,0.0)).xyz);
	gl_Position = gl_Position + mul(vec4(normal.xy,0.0,0.0),outline);
	v_bc = vec3(a_color0.x, a_color0.y, a_color0.z);
}
