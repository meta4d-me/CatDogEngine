$input a_position, a_normal, a_color0

#include "../common/common.sh"

void main()
{
	float outline = 0.5;
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

	vec3 normal = normalize(mul(u_modelInvTrans, vec4(a_normal, 0.0)).xyz);;
	normal = normalize(mul(u_view,vec4(normal,0.0)).xyz);
	gl_Position = gl_Position + mul(vec4(normal.xy,0.0,0.0),outline);
}
