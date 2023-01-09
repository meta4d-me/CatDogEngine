struct Attribute
{
	float3 a_position : POSITION;
	float3 a_normal : NORMAL;
	float3 a_tangent : TANGENT;
	float2 a_texcoord0 : TEXCOORD0;
};

struct Varying
{
	float4 gl_Position : SV_POSITION;
	float3x3 v_TBN : TEXCOORD4;
	float3 v_normal : NORMAL;
	float2 v_texcoord0 : TEXCOORD0;
	float3 v_worldPos : TEXCOORD1;
};

float4x4 mtxFromRows(float4 _0, float4 _1, float4 _2, float4 _3)
{
	return float4x4(_0, _1, _2, _3);
}
float4x4 mtxFromCols(float4 _0, float4 _1, float4 _2, float4 _3)
{
	return transpose(float4x4(_0, _1, _2, _3) );
}
float3x3 mtxFromRows(float3 _0, float3 _1, float3 _2)
{
	return float3x3(_0, _1, _2);
}
float3x3 mtxFromCols(float3 _0, float3 _1, float3 _2)
{
	return transpose(float3x3(_0, _1, _2) );
}

uniform float4 u_viewRect;
uniform float4 u_viewTexel;
uniform float4x4 u_view;
uniform float4x4 u_invView;
uniform float4x4 u_proj;
uniform float4x4 u_invProj;
uniform float4x4 u_viewProj;
uniform float4x4 u_invViewProj;
uniform float4x4 u_model[32];
uniform float4x4 u_modelView;
uniform float4x4 u_modelViewProj;
uniform float4x4 u_modelInvTrans;
uniform float4 u_alphaRef4;

Varying main(Attribute input)
{
	Varying output;
	output.v_worldPos = mul(u_model[0], float4(input.a_position, 1.0)).xyz;
	output.v_normal = normalize(mul(u_modelInvTrans, float4(input.a_normal, 0.0)).xyz);
	output.v_texcoord0 = input.a_texcoord0;
	
	float3 tangent = normalize(mul(u_modelInvTrans, float4(input.a_tangent, 0.0)).xyz);
	tangent = normalize(tangent - dot(tangent, output.v_normal) * output.v_normal);
	float3 biTangent = normalize(cross(output.v_normal, tangent));
	output.v_TBN = mtxFromCols(tangent, biTangent, output.v_normal);
	
	output.gl_Position = mul(u_modelViewProj, float4(input.a_position, 1.0));
	return output;
}
