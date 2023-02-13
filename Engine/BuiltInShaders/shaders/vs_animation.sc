$input a_position, a_indices, a_weight
$output v_worldPos, v_boneIDs, v_boneWeights

#include "../common/common.sh"
#include "uniforms.sh"

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position, 1.0));
	
	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_boneIDs = a_indices;
	v_boneWeights = a_weight;
}
