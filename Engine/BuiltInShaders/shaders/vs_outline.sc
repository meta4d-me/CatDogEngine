$input a_position, a_normal, a_color0

#include "../common/common.sh"

uniform vec4 u_outLineSize;

void main()
{
	float outline = u_outLineSize.x;
	vec4 position = mul(u_modelViewProj, vec4(a_position, 1.0));
	vec3 normal = normalize(mul(u_modelInvTrans, vec4(a_normal, 0.0)).xyz);;
	normal = normalize(mul(u_view,vec4(normal,0.0)).xyz);

	// Treat the stroke width according to the screen aspect ratio.
	normal.x *= u_viewRect.w / u_viewRect.z;
	gl_Position = position + mul(vec4(normal.xy, 0.0 ,0.0),outline);
}
