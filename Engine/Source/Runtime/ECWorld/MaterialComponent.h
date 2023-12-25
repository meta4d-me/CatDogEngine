#pragma once

#include "Core/StringCrc.h"
#include "ECWorld/SkyComponent.h"
#include "Material/ShaderSchema.h"
#include "Scene/MaterialTextureType.h"
#include "Scene/Texture.h"

#include <cstdint>
#include <map>
#include <optional>
#include <unordered_set>
#include <variant>
#include <vector>

namespace bgfx
{

struct Memory;

}

namespace cd
{

class Material;

}

namespace engine
{

class MaterialType;
class RenderContext;

class MaterialComponent final
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("MaterialComponent");
		return className;
	}

	using TextureBlob = std::vector<std::byte>;
	struct TextureInfo
	{
		static constexpr uint16_t BGFXInvalidHandle = (1 << 16) - 1;

		const bgfx::Memory* data;
		uint64_t flag;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		cd::TextureFormat format;
		cd::Vec2f uvOffset = cd::Vec2f::Zero();
		cd::Vec2f uvScale = cd::Vec2f::One();
		uint16_t samplerHandle = BGFXInvalidHandle;
		uint16_t textureHandle = BGFXInvalidHandle;
		uint8_t slot;
		uint8_t mipCount;

		// TODO : Improve TextureInfo 
		cd::Vec2f& GetUVOffset() { return uvOffset; }
		const cd::Vec2f& GetUVOffset() const { return uvOffset; }
		cd::Vec2f& GetUVScale() { return uvScale; }
		const cd::Vec2f& GetUVScale() const  { return uvScale; }
		void SetUVOffset(const cd::Vec2f& offset) { uvOffset = offset; }
		void SetUVScale(const cd::Vec2f& scale) { uvScale = scale; }
	};

	struct PropertyGroup
	{
		TextureInfo textureInfo;
		bool useTexture;
		std::variant<float, cd::Vec3f, cd::Vec4f> factor;
	};

public:
	MaterialComponent() = default;
	MaterialComponent(const MaterialComponent&) = default;
	MaterialComponent& operator=(const MaterialComponent&) = default;
	MaterialComponent(MaterialComponent&&) = default;
	MaterialComponent& operator=(MaterialComponent&&) = default;
	~MaterialComponent() = default;

	void Init();

	void SetMaterialData(const cd::Material* pMaterialData) { m_pMaterialData = pMaterialData; }
	cd::Material* GetMaterialData() { return const_cast<cd::Material*>(m_pMaterialData); }
	const cd::Material* GetMaterialData() const { return m_pMaterialData; }

	void SetMaterialType(const engine::MaterialType* pMaterialType) { m_pMaterialType = pMaterialType; }
	const engine::MaterialType* GetMaterialType() const { return m_pMaterialType; }

	void Reset();
	void Build();

	// Basic data.
	void SetName(std::string name) { m_name = cd::MoveTemp(name); }
	std::string& GetName() { return m_name; }
	const std::string& GetName() const { return m_name; }
	const std::string& GetShaderProgramName() const;

	// Uber shader data.
	void ActivateShaderFeature(ShaderFeature feature);
	void DeactivateShaderFeature(ShaderFeature feature);
	void SetShaderFeatures(std::set<ShaderFeature> options) { m_shaderFeatures = cd::MoveTemp(m_shaderFeatures); }
	std::set<ShaderFeature>& GetShaderFeatures() { return m_shaderFeatures; }
	const std::set<ShaderFeature>& GetShaderFeatures() const { return m_shaderFeatures; }
	const std::string& GetFeaturesCombine();

	// Texture data.
	void AddTextureBlob(cd::MaterialTextureType textureType, cd::TextureFormat textureFormat, cd::TextureMapMode uMapMode, cd::TextureMapMode vMapMode, TextureBlob textureBlob, uint32_t width, uint32_t height, uint32_t depth = 1);
	void AddTextureFileBlob(cd::MaterialTextureType textureType, const cd::Material* pMaterial, const cd::Texture& texture, TextureBlob textureBlob);

	const std::map<cd::MaterialPropertyGroup, PropertyGroup>& GetPropertyGroups() const { return m_propertyGroups; }
	PropertyGroup* GetPropertyGroup(cd::MaterialPropertyGroup propertyGroup);
	const PropertyGroup* GetPropertyGroup(cd::MaterialPropertyGroup propertyGroup) const;

	template<class T>
	void SetFactor(cd::MaterialPropertyGroup propertyGroup, T factor)
	{
		auto it = m_propertyGroups.find(propertyGroup);
		if (m_propertyGroups.end() == it)
		{
			return;
		}

		if (auto pValue = std::get_if<T>(&(it->second.factor)); pValue)
		{
			*pValue = cd::MoveTemp(factor);
		}
	}

	template<class T>
	T* GetFactor(cd::MaterialPropertyGroup propertyGroup)
	{
		auto it = m_propertyGroups.find(propertyGroup);
		if (m_propertyGroups.end() == it)
		{
			return nullptr;
		}

		return std::get_if<T>(&(it->second.factor));
	}

	template<class T>
	const T* GetFactor(cd::MaterialPropertyGroup propertyGroup) const
	{
		auto it = m_propertyGroups.find(propertyGroup);
		if (m_propertyGroups.end() == it)
		{
			return nullptr;
		}

		return std::get_if<T>(&(it->second.factor));
	}

	// Cull parameters. 
	void SetTwoSided(bool value) { m_twoSided = value; }
	bool& GetTwoSided() { return m_twoSided; }
	bool GetTwoSided() const { return m_twoSided; }

	// Blend parameters.
	void SetBlendMode(cd::BlendMode blendMode) { m_blendMode = blendMode; }
	cd::BlendMode& GetBlendMode() { return m_blendMode; }
	cd::BlendMode GetBlendMode() const { return m_blendMode; }

	void SetAlphaCutOff(float value) { m_alphaCutOff = value; }
	float& GetAlphaCutOff() { return m_alphaCutOff; }
	float GetAlphaCutOff() const { return m_alphaCutOff; }

private:
	// Input
	const cd::Material* m_pMaterialData = nullptr;
	const engine::MaterialType* m_pMaterialType = nullptr;

	std::string m_name;
	bool m_twoSided;
	cd::BlendMode m_blendMode;
	float m_alphaCutOff;

	bool m_isShaderFeatureDirty = false;
	std::set<ShaderFeature> m_shaderFeatures;
	std::string m_featureCombine;

	std::vector<TextureBlob> m_cacheTextureBlobs;

	// Output
	std::map<cd::MaterialTextureType, PropertyGroup> m_propertyGroups;
};

}
