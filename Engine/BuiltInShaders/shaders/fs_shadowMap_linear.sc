$input v_worldPos

#include "../common/common.sh"
#include "../common/LightSource.sh"

uniform vec4 u_lightWorldPos_farPlane;

void main()
{
    float diatance = length(v_worldPos - u_lightWorldPos_farPlane.xyz);
    gl_FragColor= vec4(diatance/u_lightWorldPos_farPlane.w, 0.0, 0.0, 1.0);//
}