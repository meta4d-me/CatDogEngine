$input a_position, a_color0, a_texcoord0, i_data0, i_data1 ,i_data2 ,i_data3 ,i_data4
$output v_color0, v_texcoord0

#include "../common/common.sh"

uniform vec4 u_cameraPos;
uniform vec4 u_particleUp;

void main()
{
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);

    vec4 forward = u_cameraPos - model[3];
    forward = mul(model,forward);

    vec4 right = vec4(
    u_particleUp.y * forward.z - u_particleUp.z * forward.y,
    u_particleUp.z * forward.x - u_particleUp.x * forward.z,
    u_particleUp.x * forward.y - u_particleUp.y * forward.x,
    1.0
    );

    float pitch = -atan2(forward.y, sqrt(forward.x * forward.x + forward.z * forward.z)); 
    float yaw = -atan2(right.z, right.x);
    float roll = atan2(right.x, -right.y);
    
    mat4 pitchMatrix = mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, cos(pitch), -sin(pitch), 0.0,
        0.0, sin(pitch), cos(pitch), 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    mat4 yawMatrix = mat4(
        cos(yaw), 0.0, sin(yaw), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(yaw), 0.0, cos(yaw), 0.0,
        0.0, 0.0, 0.0, 1.0
    );

    mat4 rollMatrix = mat4(
        cos(roll), -sin(roll), 0.0, 0.0,
        sin(roll), cos(roll), 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
    mat4 rotationMatrix = mul(pitchMatrix,mul(yawMatrix,rollMatrix));
    model = mul(model,rotationMatrix);

	vec4 worldPos = mul(model,vec4(a_position,1.0));
	gl_Position = mul(u_viewProj, worldPos);
	v_color0    = a_color0*i_data4;
	v_texcoord0 = a_texcoord0;
}