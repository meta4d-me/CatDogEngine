#pragma once

#include "IResource.h"

#include <vector>

namespace cd
{

enum class MaterialPropertyGroup;
class Texture;
enum class TextureMapMode;

}

namespace engine
{

class TextureResource : public IResource
{
public:
	using TextureRawData = std::vector<std::byte>;

public:
	TextureResource();
	TextureResource(const TextureResource&) = default;
	TextureResource& operator=(const TextureResource&) = default;
	TextureResource(TextureResource&&) = default;
	TextureResource& operator=(TextureResource&&) = default;
	virtual ~TextureResource();

	void UpdateTextureType(cd::MaterialPropertyGroup textureType);
	void UpdateUVMapMode(cd::TextureMapMode u, cd::TextureMapMode v);
	virtual void Update() override;

	const cd::Texture* GetTextureAsset() const { return m_pTextureAsset; }
	void SetTextureAsset(const cd::Texture* pTextureAsset);

	uint16_t GetTextureHandle() const { return m_textureHandle; }

private:
	uint64_t GetTextureFlags() const;
	void BuildTextureHandle();
	void DestroyTextureHandle();

private:
	// Asset
	const cd::Texture* m_pTextureAsset = nullptr;

	// Runtime
	bool m_enableSRGB = false;
	cd::TextureMapMode m_uvMapMode[2];

	// CPU
	TextureRawData m_textureRawData;
	void* m_textureImageData = nullptr;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_textureHandle = UINT16_MAX;
};

}