#pragma once

#include "IResource.h"

#include <vector>
#include <string>

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

	virtual void Update() override;
	virtual void Reset() override;

	// TODO : Move resource builder to engine and aync build not to block main thread.
	void SetDDSBuiltTexturePath(std::string ddsFilePath);

	void UpdateTextureType(cd::MaterialPropertyGroup textureType);
	void UpdateUVMapMode(cd::TextureMapMode u, cd::TextureMapMode v);

	const cd::Texture* GetTextureAsset() const { return m_pTextureAsset; }
	void SetTextureAsset(const cd::Texture* pTextureAsset);

	uint16_t GetSamplerHandle() const { return m_samplerHandle; }
	uint16_t GetTextureHandle() const { return m_textureHandle; }

private:
	uint64_t GetTextureFlags() const;

	void BuildSamplerHandle();
	void BuildTextureHandle();

	void ClearTextureData();
	void FreeTextureData();

	void DestroySamplerHandle();
	void DestroyTextureHandle();

private:
	// Asset
	const cd::Texture* m_pTextureAsset = nullptr;
	std::string m_ddsFilePath;

	// Runtime
	bool m_enableSRGB = false;
	cd::TextureMapMode m_uvMapMode[2];

	// CPU
	TextureRawData m_textureRawData;
	void* m_textureImageData = nullptr;
	uint32_t m_recycleCount = 0;

	// GPU
	uint16_t m_samplerHandle = UINT16_MAX;
	uint16_t m_textureHandle = UINT16_MAX;
};

}