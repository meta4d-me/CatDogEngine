$input a_position//, a_color1

#include "../common/common.sh"

uniform vec4 u_shapeRange;

void main()
{
	gl_Position = mul(u_modelViewProj, vec4(a_position*u_shapeRange.xyz, 1.0));
}