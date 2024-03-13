$input a_position, a_normal, a_tangent, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0, v_TBN, v_color0

#include "../common/common.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));

	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_color0 = mul(u_modelView, vec4(a_position, 1.0));
	v_normal     = normalize(mul(u_modelInvTrans, vec4(a_normal, 0.0)).xyz);
	vec3 tangent = normalize(mul(u_modelInvTrans, vec4(a_tangent, 0.0)).xyz);
	
	// re-orthogonalize T with respect to N
	tangent        = normalize(tangent - dot(tangent, v_normal) * v_normal);
	vec3 biTangent = normalize(cross(v_normal, tangent));
	
	// TBN
	v_TBN = mtxFromCols(tangent, biTangent, v_normal);
	v_texcoord0 = a_texcoord0;
}
