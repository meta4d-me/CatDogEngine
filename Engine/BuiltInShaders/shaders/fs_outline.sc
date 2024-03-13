#include "../common/common.sh"
#include "../common/Material.sh"

uniform vec4 u_outLineColor;

void main()
{
	gl_FragColor = vec4(u_outLineColor.xyz, 1.0);
}