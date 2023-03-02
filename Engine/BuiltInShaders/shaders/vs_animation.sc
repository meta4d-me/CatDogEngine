$input a_position, a_indices, a_weight
$output v_worldPos

#include "../common/common.sh"
#include "uniforms.sh"

uniform mat4 u_boneMatrices[65];

void main()
{
	mat4 boneTransform = u_boneMatrices[a_indices[0]] * a_weight[0];
	boneTransform += u_boneMatrices[a_indices[1]] * a_weight[1];
	boneTransform += u_boneMatrices[a_indices[2]] * a_weight[2];
	boneTransform += u_boneMatrices[a_indices[3]] * a_weight[3];
	
	vec4 localPosition = mul(boneTransform, vec4(a_position, 1.0));
	gl_Position = mul(u_modelViewProj, localPosition);
	
	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
}