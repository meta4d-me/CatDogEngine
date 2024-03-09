//------------------------------------------------------------------------//
// @brief Returns Material object whose members' values are determined by //
// the bound texture if macro definitions are turned on,                  //
// otherwise they are default values.                                     //
//                                                                        //
// Material GetMaterial(vec2 uv, vec3 normal, mat3 TBN);                  //
//------------------------------------------------------------------------//

// To reuse Material.sh, we reserve the first 4 slots (0 - 3) in U_BaseSlot.sh.
#include "../UniformDefines/U_BaseSlot.sh"

struct Material {
	vec3 albedo;
	vec3 normal;
	float occlusion;
	float roughness;
	float metallic;
	vec3 F0;
	float opacity;
	vec3 emissive;
};

uniform vec4 u_outLineColor;
uniform vec4 u_outLineSize;
uniform vec4 u_albedoColor;
uniform vec4 u_metallicRoughnessFactor;
uniform vec4 u_albedoUVOffsetAndScale;
uniform vec4 u_alphaCutOff;
uniform vec4 u_dividLine;
uniform vec4 u_specular;
uniform vec4 u_firstShadowColor;
uniform vec4 u_secondShadowColor;
uniform vec4 u_rimLightColor;
uniform vec4 u_rimLight;

#if defined(ALBEDOMAP)
SAMPLER2D(s_texBaseColor, ALBEDO_MAP_SLOT);

vec4 SampleAlbedoTexture(vec2 uv) {
	// We assume that albedo texture is already in linear space.
	return texture2D(s_texBaseColor, uv);
}
#endif

#if defined(NORMALMAP)
SAMPLER2D(s_texNormal, NORMAL_MAP_SLOT);

vec3 SampleNormalTexture(vec2 uv, mat3 TBN) {
	// We assume that normal texture is already in linear space.
	vec3 normalTexture = normalize(texture2D(s_texNormal, uv).xyz * 2.0 - 1.0);
	return normalize(mul(TBN, normalTexture));
}
#endif

#if defined(ORMMAP)
// ORM means that the three attributes of
// Occlusion, Roughness and Metallic are located in the
// R, G and B channels of the ORM texture.
SAMPLER2D(s_texORM, ORM_MAP_SLOT);

vec3 SampleORMTexture(vec2 uv) {
	// We assume that ORM texture is already in linear space.
	vec3 orm = texture2D(s_texORM, uv).xyz;
	
	// Clamp rouphness.
	orm.y = clamp(orm.y, 0.04, 1.0);
	
	return orm;
}
#endif

#if defined(EMISSIVEMAP)
SAMPLER2D(s_texEmissive, EMISSIVE_MAP_SLOT);

vec3 SampleEmissiveTexture(vec2 uv) {
	// We assume that emissive texture is already in linear space.
	return texture2D(s_texEmissive, uv).xyz;
}
#endif

Material CreateMaterial() {
	Material material;
	material.albedo = vec3(1.0, 1.0, 1.0);
	material.normal = vec3(0.0, 1.0, 0.0);
	material.occlusion = 1.0;
	material.roughness = 0.9;
	material.metallic = 0.1;
	material.F0 = vec3(0.04, 0.04, 0.04);
	material.opacity = 1.0;
	material.emissive = vec3_splat(0.0);
	return material;
}

Material GetMaterial(vec2 uv, vec3 normal, mat3 TBN) {
	Material material = CreateMaterial();

	vec2 uvOffset = vec2(u_albedoUVOffsetAndScale.x, u_albedoUVOffsetAndScale.y);
	vec2 uvScale = vec2(u_albedoUVOffsetAndScale.z, u_albedoUVOffsetAndScale.w);
	vec2 albedoUV = uv * uvScale + uvOffset;
	
#if defined(ALBEDOMAP)
	vec4 albedoTexture = SampleAlbedoTexture(albedoUV);
	material.albedo = albedoTexture.xyz;
	material.opacity = albedoTexture.w;
#endif
	material.albedo *= u_albedoColor.xyz;

#if defined(NORMALMAP)
	// Same to unity standard PBR, let normal uv same with albedo uv.
	material.normal = SampleNormalTexture(albedoUV, TBN);
#else
	material.normal = normal;
#endif
	
#if defined(ORMMAP)
	vec3 orm = SampleORMTexture(uv);
	material.occlusion = orm.x;
	material.roughness = orm.y;
	material.metallic = orm.z;
#else
	material.roughness = u_metallicRoughnessFactor.y;
	material.metallic = u_metallicRoughnessFactor.x;
#endif

#if defined(EMISSIVEMAP)
	material.emissive = SampleEmissiveTexture(uv);
#endif
	
	material.F0 = mix(vec3_splat(0.04), material.albedo, material.metallic);
	
	return material;
}
