#include "SceneRenderer.h"

#include "BgfxConsumer.h"
#include "Producer/CatDogProducer.h"
#include "Processor/Processor.h"
#include "Scene/Texture.h"
#include "SwapChain.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

namespace
{

constexpr uint32_t MakeRBGA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return (red << 24) + (green << 16) + (blue << 8) + alpha;
}

struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;

	static void init()
	{
		ms_layout
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
	};

	static bgfx::VertexLayout ms_layout;
};

static PosColorVertex s_cubeVertices[] =
{
	{-1.0f,  1.0f,  1.0f, 0xff000000 },
	{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
	{-1.0f, -1.0f,  1.0f, 0xff00ff00 },
	{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
	{-1.0f,  1.0f, -1.0f, 0xffff0000 },
	{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
	{-1.0f, -1.0f, -1.0f, 0xffffff00 },
	{ 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

bgfx::VertexLayout PosColorVertex::ms_layout;
static bgfx::VertexBufferHandle s_vbh;
static bgfx::IndexBufferHandle s_ibh;
static bgfx::ShaderHandle s_vsh;
static bgfx::ShaderHandle s_fsh;
static bgfx::ProgramHandle s_ph;

}

namespace engine
{

void SceneRenderer::LoadSceneData(std::string sceneFilePath)
{
	// CatDogProducer can parse .catdog.bin format file to SceneDatabase in memory.
	// BgfxConsumer is used to translate SceneDatabase to data which bgfx api can use directly.
	cdtools::CatDogProducer cdProducer(std::move(sceneFilePath));
	cdtools::BgfxConsumer bgfxConsumer("");
	cdtools::Processor processor(&cdProducer, &bgfxConsumer);
	processor.Run();

	// Start creating bgfx resources from RenderDataContext
	RenderDataContext renderDataContext = bgfxConsumer.GetRenderDataContext();

}

void SceneRenderer::Init()
{
	PosColorVertex::init();

	s_vbh = bgfx::createVertexBuffer(bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices)), PosColorVertex::ms_layout);
	s_ibh = bgfx::createIndexBuffer(bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList)));

	s_vsh = Renderer::LoadShader("vs_cubes");
	s_fsh = Renderer::LoadShader("fs_cubes");

	s_ph = bgfx::createProgram(s_vsh, s_fsh, true);
}

void SceneRenderer::Render(float deltaTime)
{
	Renderer::Render(deltaTime);

	const SwapChain* pSwapChain = GetSwapChain();

	const bx::Vec3 at = { 0.0f, 0.0f,   0.0f };
	const bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };
	float view[16];
	bx::mtxLookAt(view, eye, at);

	float proj[16];
	bx::mtxProj(proj, 60.0f, pSwapChain->GetAspect(), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(0, view, proj);
	bgfx::setViewRect(GetViewID(), 0, 0, pSwapChain->GetWidth(), pSwapChain->GetHeight());
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, MakeRBGA(128, 0, 0, 255), 1.0f, 0);

	uint64_t state = 0
		| BGFX_STATE_WRITE_R
		| BGFX_STATE_WRITE_G
		| BGFX_STATE_WRITE_B
		| BGFX_STATE_WRITE_A
		| BGFX_STATE_WRITE_Z
		| BGFX_STATE_DEPTH_TEST_LESS
		| BGFX_STATE_CULL_CW
		| BGFX_STATE_MSAA
		;

	for (uint32_t yy = 0; yy < 11; ++yy)
	{
		for (uint32_t xx = 0; xx < 11; ++xx)
		{
			float mtx[16];
			bx::mtxRotateXY(mtx, xx * 0.21f, yy * 0.37f);
			mtx[12] = -15.0f + float(xx) * 3.0f;
			mtx[13] = -15.0f + float(yy) * 3.0f;
			mtx[14] = 0.0f;

			// Set model matrix for rendering.
			bgfx::setTransform(mtx);
			bgfx::setVertexBuffer(0, s_vbh);
			bgfx::setIndexBuffer(s_ibh);
			bgfx::setState(state);
			bgfx::submit(0, s_ph);
		}
	}
}

}