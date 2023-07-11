$input a_position, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0, v_alphaMapTexCoord

#include "../common/common.sh"

uniform vec4 u_SectorOrigin;
uniform vec4 u_SectorDimension;

ISAMPLER2D(s_elevationMap, 1);

float getElevation(vec3 worldPosition)
{
    int u = worldPosition.x - u_SectorOrigin.x;
    int v = worldPosition.z - u_SectorOrigin.z;
    ivec4 elevation = texelFetch(s_elevationMap, ivec2(u, v), 0);
    return elevation.r;
}

void main()
{
    float height = getElevation(a_position);
	gl_Position = mul(u_modelViewProj, vec4(a_position.x, height, a_position.z, 1.0));
	v_worldPos = mul(u_model[0], vec4(a_position, 1.0)).xyz;
	v_normal = normalize(mul(u_modelInvTrans, vec4(0.0, 1.0, 0.0, 0.0)).xyz);
	v_texcoord0 = a_texcoord0;
    v_alphaMapTexCoord = vec2((a_position.x - u_SectorOrigin.x) / u_SectorDimension.x, (a_position.z - u_SectorOrigin.z) / u_SectorDimension.y);
}
