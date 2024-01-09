$input a_position, a_color0, a_texcoord0, i_data0, i_data1 ,i_data2 ,i_data3 ,i_data4
$output v_color0, v_texcoord0

#include "../common/common.sh"

uniform vec4 u_cameraPos;
uniform vec4 u_particleUp;

void main()
{
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

    vec4 lookforward = u_cameraPos - model[3];
    vec4 right = vec4(
    lookforward.y * u_particleUp.z - lookforward.z * u_particleUp.y,
    lookforward.z * u_particleUp.x - lookforward.x * u_particleUp.z,
    lookforward.x * u_particleUp.y - lookforward.y * u_particleUp.x,
    0.0
    );

    float pitch = atan2(lookforward.y, sqrt(lookforward.x * lookforward.x + lookforward.z * lookforward.z)); 
    float yaw = atan2(right.z, right.x);
    float roll = atan2(right.x, -right.y); 

    mat4 rotationMatrix;
    rotationMatrix[0] = vec4(
        cos(yaw) * cos(roll)-sin(pitch)*sin(roll)*sin(yaw),
        -cos(pitch)*sin(roll),
        cos(roll)*sin(yaw) + cos(yaw)*sin(pitch)*sin(roll),
        0.0f
    );
    rotationMatrix[1] = vec4(
        cos(roll)*sin(pitch)*sin(yaw)+cos(yaw)*sin(roll),
        cos(pitch)*cos(roll),
        sin(yaw)*sin(roll) - cos(yaw)*cos(roll)*sin(pitch),
        0.0f
    );
    rotationMatrix[2] = vec4(
        -cos(pitch)*sin(yaw),
        sin(pitch),
        cos(pitch)*cos(yaw),
        0.0f
    );
    rotationMatrix[3] = vec4(
        0.0f,0.0f,0.0f,1.0f
    );

    model = mul(model,rotationMatrix);

	vec4 worldPos = mul(model,vec4(a_position,1.0));
	gl_Position = mul(u_viewProj, worldPos);
	v_color0    = a_color0*i_data4;
	v_texcoord0 = a_texcoord0;
}