#if defined(PARTICLEINSTANCE)
$input a_position, a_color0, a_texcoord0, i_data0, i_data1 ,i_data2 ,i_data3 ,i_data4
#else
$input a_position, a_color0, a_texcoord0
#endif

$output v_color0, v_texcoord0

#if defined(PARTICLEINSTANCE)
#include "../common/common.sh"
#else
#include "../common/common.sh"
uniform vec4 u_particleColor;
#endif

void main()
{
	#if defined(PARTICLEINSTANCE)
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);
	vec4 worldPos = mul(model,vec4(a_position,1.0));
	gl_Position = mul(u_viewProj, worldPos);
	v_color0    = a_color0*i_data4;
	#else
	gl_Position = mul(u_modelViewProj, vec4(a_position,1.0));
	v_color0    = a_color0 * u_particleColor;
	#endif

	v_texcoord0 = a_texcoord0;
}