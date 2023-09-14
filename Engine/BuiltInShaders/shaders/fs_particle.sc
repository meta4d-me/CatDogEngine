$input v_color0
#include "../common/common.sh"


void main()
{
	gl_FragColor = float4(v_color0.rgb,1.0f);
}