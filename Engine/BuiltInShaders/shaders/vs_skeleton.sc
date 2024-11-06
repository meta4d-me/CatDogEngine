$input a_position, a_indices

#include "../common/common.sh"

uniform mat4 u_boneMatrices[128];

void main()
{
	mat4 boneTransform = u_boneMatrices[a_indices[0]];
	vec4 localPosition = mul(boneTransform, vec4(a_position, 1.0));
	gl_Position = mul(u_modelViewProj, localPosition);
}