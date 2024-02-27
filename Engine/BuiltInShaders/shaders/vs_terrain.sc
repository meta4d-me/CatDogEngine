$input a_position, a_normal, a_tangent, a_texcoord0
$output v_worldPos, v_normal, v_texcoord0, v_TBN, v_color0

#include "../common/common.sh"

#include "../UniformDefines/U_Terrain.sh"

SAMPLER2D(s_texElevation, TERRAIN_ELEVATION_MAP_SLOT);

void main()
{
	vec2 uvElevation = vec2(a_texcoord0.x*4/129,a_texcoord0.y*4/129);
	float elevation = texture2DLod(s_texElevation, uvElevation, 0).x;

	vec2 uvRElevation = vec2((a_texcoord0.x*4+1)/129,(a_texcoord0.y*4)/129);
	vec2 uvLElevation = vec2((a_texcoord0.x*4-1)/129,(a_texcoord0.y*4)/129);
	vec2 uvTElevation = vec2((a_texcoord0.x*4)/129,(a_texcoord0.y*4+1)/129);
	vec2 uvBElevation = vec2((a_texcoord0.x*4)/129,(a_texcoord0.y*4-1)/129);
	float elevationR = texture2DLod(s_texElevation, uvRElevation, 0).x;
	float elevationL = texture2DLod(s_texElevation, uvLElevation, 0).x;
	float elevationT = texture2DLod(s_texElevation, uvTElevation, 0).x;
	float elevationB = texture2DLod(s_texElevation, uvBElevation, 0).x;

	vec3 N1 = cross(vec3(a_position.x+1, elevationR, a_position.z),
					vec3(a_position.x, elevationT, a_position.z+1));
	vec3 N2 = cross(vec3(a_position.x, elevationT, a_position.z+1),
					vec3(a_position.x-1, elevationL, a_position.z));
	vec3 N3 = cross(vec3(a_position.x-1, elevationL, a_position.z),
					vec3(a_position.x, elevationB, a_position.z-1));
	vec3 N4 = cross(vec3(a_position.x, elevationB, a_position.z-1),
					vec3(a_position.x+1, elevationR, a_position.z));

	gl_Position = mul(u_modelViewProj, vec4(a_position.x, elevation, a_position.z, 1.0));
	v_worldPos = mul(u_model[0], vec4(a_position.x, elevation, a_position.z, 1.0)).xyz;
	v_color0 = mul(u_modelView, vec4(a_position, 1.0));
	
	v_normal     = -normalize(N1 + N2 + N3 +N4);
	vec3 tangent = normalize(mul(u_modelInvTrans, vec4(a_tangent, 0.0)).xyz);
	
	// re-orthogonalize T with respect to N
	tangent        = normalize(tangent - dot(tangent, v_normal) * v_normal);
	vec3 biTangent = normalize(cross(v_normal, tangent));
	
	// TBN
	v_TBN = mtxFromCols(tangent, biTangent, v_normal);
	
	v_texcoord0 = a_texcoord0;
}
