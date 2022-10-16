#include "Renderer.h"

#include "Camera.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>
#include <bimg/decode.h>
#include <bx/allocator.h>

#include <fstream>

namespace
{

static bx::AllocatorI* GetResourceAllocator()
{
	static bx::DefaultAllocator s_allocator;
	return &s_allocator;
}

static void imageReleaseCb(void* _ptr, void* _userData)
{
	BX_UNUSED(_ptr);
	bimg::ImageContainer* imageContainer = (bimg::ImageContainer*)_userData;
	bimg::imageFree(imageContainer);
}

}

namespace engine
{

Renderer::Renderer(uint16_t viewID, SwapChain* pSwapChain, GBuffer* pGBuffer, FlybyCamera* pCamera) :
	m_viewID(viewID),
	m_pSwapChain(pSwapChain),
	m_pGBuffer(pGBuffer),
	m_pFlybyCamera(pCamera)
{
}

void Renderer::Render(float deltaTime)
{
}

bgfx::TextureHandle Renderer::LoadTexture(std::string filePath, uint64_t flags)
{
	std::ifstream fin(CDENGINE_RESOURCES_ROOT_PATH + filePath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return bgfx::TextureHandle(bgfx::kInvalidHandle);
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	uint8_t* pRawData = new uint8_t[fileSize];
	fin.read(reinterpret_cast<char*>(pRawData), fileSize);
	fin.close();

	bimg::ImageContainer* imageContainer = bimg::imageParse(GetResourceAllocator(), pRawData, static_cast<uint32_t>(fileSize));
	const bgfx::Memory* mem = bgfx::makeRef(
		imageContainer->m_data
		, imageContainer->m_size
		, imageReleaseCb
		, imageContainer
	);

	delete[] pRawData;
	pRawData = nullptr;

	bgfx::TextureHandle handle(bgfx::kInvalidHandle);
	if (imageContainer->m_cubeMap)
	{
		handle = bgfx::createTextureCube(
			uint16_t(imageContainer->m_width)
			, 1 < imageContainer->m_numMips
			, imageContainer->m_numLayers
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}
	else if (1 < imageContainer->m_depth)
	{
		handle = bgfx::createTexture3D(
			uint16_t(imageContainer->m_width)
			, uint16_t(imageContainer->m_height)
			, uint16_t(imageContainer->m_depth)
			, 1 < imageContainer->m_numMips
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}
	else if (bgfx::isTextureValid(0, false, imageContainer->m_numLayers, bgfx::TextureFormat::Enum(imageContainer->m_format), flags))
	{
		handle = bgfx::createTexture2D(
			uint16_t(imageContainer->m_width)
			, uint16_t(imageContainer->m_height)
			, 1 < imageContainer->m_numMips
			, imageContainer->m_numLayers
			, bgfx::TextureFormat::Enum(imageContainer->m_format)
			, flags
			, mem
		);
	}

	if (bgfx::isValid(handle))
	{
		bgfx::setName(handle, filePath.c_str());
	}

	return handle;
}

bgfx::ShaderHandle Renderer::LoadShader(std::string filePath)
{
	std::ifstream fin(CDENGINE_RESOURCES_ROOT_PATH + filePath, std::ios::in | std::ios::binary);
	if (!fin.is_open())
	{
		return bgfx::ShaderHandle(bgfx::kInvalidHandle);
	}

	fin.seekg(0L, std::ios::end);
	size_t fileSize = fin.tellg();
	fin.seekg(0L, std::ios::beg);
	uint8_t* pRawData = new uint8_t[fileSize];
	fin.read(reinterpret_cast<char*>(pRawData), fileSize);
	fin.close();

	const bgfx::Memory* pMemory = bgfx::makeRef(pRawData, static_cast<uint32_t>(fileSize));
	bgfx::ShaderHandle handle = bgfx::createShader(pMemory);
	bgfx::setName(handle, filePath.c_str());

	return handle;
}

struct PosColorTexCoord0Vertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_rgba;
	float m_u;
	float m_v;

	static void init() {
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();
	}

	static bgfx::VertexLayout ms_layout;
};

static bool bInitScreenSpaceQuadVertexLayout = false;
bgfx::VertexLayout PosColorTexCoord0Vertex::ms_layout;

void Renderer::ScreenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft, float _width, float _height)
{
	if (!bInitScreenSpaceQuadVertexLayout)
	{
		PosColorTexCoord0Vertex::init();
		bInitScreenSpaceQuadVertexLayout = true;
	}

	if (3 == bgfx::getAvailTransientVertexBuffer(3, PosColorTexCoord0Vertex::ms_layout)) {
		bgfx::TransientVertexBuffer vb;
		bgfx::allocTransientVertexBuffer(&vb, 3, PosColorTexCoord0Vertex::ms_layout);
		PosColorTexCoord0Vertex* vertex = (PosColorTexCoord0Vertex*)vb.data;

		const float zz = 0.0f;

		const float minx = -_width;
		const float maxx = _width;
		const float miny = 0.0f;
		const float maxy = _height * 2.0f;

		static float s_texelHalf = 0.0f;

		const float texelHalfW = s_texelHalf / _textureWidth;
		const float texelHalfH = s_texelHalf / _textureHeight;
		const float minu = -1.0f + texelHalfW;
		const float maxu = 1.0f + texelHalfW;

		float minv = texelHalfH;
		float maxv = 2.0f + texelHalfH;

		if (_originBottomLeft) {
			std::swap(minv, maxv);
			minv -= 1.0f;
			maxv -= 1.0f;
		}

		vertex[0].m_x = minx;
		vertex[0].m_y = miny;
		vertex[0].m_z = zz;
		vertex[0].m_rgba = 0xffffffff;
		vertex[0].m_u = minu;
		vertex[0].m_v = minv;

		vertex[1].m_x = maxx;
		vertex[1].m_y = miny;
		vertex[1].m_z = zz;
		vertex[1].m_rgba = 0xffffffff;
		vertex[1].m_u = maxu;
		vertex[1].m_v = minv;

		vertex[2].m_x = maxx;
		vertex[2].m_y = maxy;
		vertex[2].m_z = zz;
		vertex[2].m_rgba = 0xffffffff;
		vertex[2].m_u = maxu;
		vertex[2].m_v = maxv;

		bgfx::setVertexBuffer(0, &vb);
	}
}

}