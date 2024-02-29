#if defined(PARTICLEINSTANCE)
$input a_position, a_color0, a_texcoord0, i_data0, i_data1 ,i_data2 ,i_data3 ,i_data4
$output v_color0, v_texcoord0

#include "../common/common.sh"

uniform vec4 u_particlePos;
uniform vec4 u_particleScale;

void main()
{
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

    mat4 billboardMatrix;
    billboardMatrix[0] = vec4(
        u_view[0][0]*u_particleScale.x,
        u_view[1][0]*u_particleScale.x,
        u_view[2][0]*u_particleScale.x,
        0.0
    );
    billboardMatrix[1] = vec4(
        u_view[0][1]*u_particleScale.y,
        u_view[1][1]*u_particleScale.y,
        u_view[2][1]*u_particleScale.y,
        0.0
    );
    billboardMatrix[2] = vec4(
        u_view[0][2]*u_particleScale.z,
        u_view[1][2]*u_particleScale.z,
        u_view[2][2]*u_particleScale.z,
        0.0
    );
    billboardMatrix[3] = vec4(
        u_particlePos.x,
        u_particlePos.y,
        u_particlePos.z,
        1.0
    );

    model = mul(model,billboardMatrix);

	vec4 worldPos = mul(model,vec4(a_position,1.0));
	gl_Position = mul(u_viewProj, worldPos);
	v_color0    = a_color0*i_data4;
	v_texcoord0 = a_texcoord0;
}
#else
$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include "../common/common.sh"

uniform vec4 u_particleColor;

void main()
{

	gl_Position = mul(u_modelViewProj, vec4(a_position,1.0));
	v_color0    = a_color0 * u_particleColor;
	v_texcoord0 = a_texcoord0;
}
#endif