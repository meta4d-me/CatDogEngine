#include "MaterialComponent.h"

#include "Log/Log.h"
#include "Material/MaterialType.h"
#include "Scene/Material.h"

#include <cassert>
#include <unordered_map>

namespace engine
{

void MaterialComponent::Init()
{
	Reset();
	PropertyGroup propertyGroup;

	propertyGroup.useTexture = false;
	propertyGroup.factor = cd::Vec3f{ 1.0f, 1.0f, 1.0f };
	m_propertyGroups[cd::MaterialPropertyGroup::BaseColor] = cd::MoveTemp(propertyGroup);

	propertyGroup.useTexture = false;
	m_propertyGroups[cd::MaterialPropertyGroup::Normal] = cd::MoveTemp(propertyGroup);

	propertyGroup.useTexture = false;
	propertyGroup.factor = 1.0f;
	m_propertyGroups[cd::MaterialPropertyGroup::Occlusion] = cd::MoveTemp(propertyGroup);

	propertyGroup.useTexture = false;
	propertyGroup.factor = 0.9f;
	m_propertyGroups[cd::MaterialPropertyGroup::Roughness] = cd::MoveTemp(propertyGroup);

	propertyGroup.useTexture = false;
	propertyGroup.factor = 0.1f;
	m_propertyGroups[cd::MaterialPropertyGroup::Metallic] = cd::MoveTemp(propertyGroup);

	propertyGroup.useTexture = false;
	propertyGroup.factor = cd::Vec4f{ 1.0f, 1.0f, 1.0f, 1.0f };
	m_propertyGroups[cd::MaterialPropertyGroup::Emissive] = cd::MoveTemp(propertyGroup);
}

void MaterialComponent::SetMaterialData(const cd::Material* pMaterialData)
{
	m_pMaterialData = pMaterialData;
	if (m_pMaterialData)
	{
		m_name = m_pMaterialData->GetName();
	}
}

MaterialComponent::PropertyGroup* MaterialComponent::GetPropertyGroup(cd::MaterialPropertyGroup propertyGroup)
{
	auto it = m_propertyGroups.find(propertyGroup);
	if (m_propertyGroups.end() == it)
	{
		return nullptr;
	}

	return &(it->second);
}

const MaterialComponent::PropertyGroup* MaterialComponent::GetPropertyGroup(cd::MaterialPropertyGroup propertyGroup) const
{
	auto it = m_propertyGroups.find(propertyGroup);
	if (m_propertyGroups.end() == it)
	{
		return nullptr;
	}

	return &(it->second);
}

const std::string& MaterialComponent::GetShaderProgramName() const
{
	return m_pMaterialType->GetShaderSchema().GetShaderProgramName();
}

void MaterialComponent::ActivateShaderFeature(ShaderFeature feature)
{
	if (ShaderFeature::DEFAULT == feature)
	{
		return;
	}

	for (const auto& conflict : m_pMaterialType->GetShaderSchema().GetConflictFeatureSet(feature))
	{
		m_shaderFeatures.erase(conflict);
	}

	m_shaderFeatures.insert(cd::MoveTemp(feature));

	m_isShaderFeatureDirty = true;
}

void MaterialComponent::DeactivateShaderFeature(ShaderFeature feature)
{
	m_shaderFeatures.erase(feature);

	m_isShaderFeatureDirty = true;
}

const std::string& MaterialComponent::GetFeaturesCombine()
{
	if (false == m_isShaderFeatureDirty)
	{
		return m_featureCombine;
	}

	m_featureCombine = m_pMaterialType->GetShaderSchema().GetFeaturesCombine(m_shaderFeatures);
	m_isShaderFeatureDirty = false;

	return m_featureCombine;
}

void MaterialComponent::Reset()
{
	m_pMaterialData = nullptr;
	m_pMaterialType = nullptr;
	m_name.clear();
	m_twoSided = false;
	m_blendMode = cd::BlendMode::Opaque;
	m_alphaCutOff = 1.0f;
	m_isShaderFeatureDirty = false;
	m_shaderFeatures.clear();
	m_featureCombine.clear();
	m_cacheTextureBlobs.clear();
	m_propertyGroups.clear();
}

TextureResource* MaterialComponent::GetTextureResource(cd::MaterialTextureType textureType) const
{
	auto itPropertyGroup = m_propertyGroups.find(textureType);
	return itPropertyGroup != m_propertyGroups.end() ? itPropertyGroup->second.textureInfo.pTextureResource : nullptr;
}

void MaterialComponent::SetTextureResource(cd::MaterialTextureType textureType, cd::Vec2f uvOffset, cd::Vec2f uvScale, TextureResource* pTextureResource)
{
	std::optional<uint8_t> optTextureSlot = m_pMaterialType->GetTextureSlot(textureType);
	if (!optTextureSlot.has_value())
	{
		return;
	}

	PropertyGroup& propertyGroup = m_propertyGroups[textureType];
	propertyGroup.useTexture = true;

	TextureInfo& textureInfo = propertyGroup.textureInfo;
	textureInfo.slot = optTextureSlot.value();
	textureInfo.pTextureResource = pTextureResource;
	textureInfo.uvScale = uvScale;
	textureInfo.uvOffset = uvOffset;
}

}
