#include "SkyRenderer.h"

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <fstream>

namespace
{

constexpr uint32_t MakeRBGA(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha)
{
	return (red << 24) + (green << 16) + (blue << 8) + alpha;
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
bgfx::VertexLayout PosColorTexCoord0Vertex::ms_layout;

void ScreenSpaceQuad(float _textureWidth, float _textureHeight, bool _originBottomLeft = false, float _width = 1.0f, float _height = 1.0f)
{
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

namespace engine::Rendering
{

void SkyRenderer::Init()
{
	PosColorTexCoord0Vertex::init();

	m_uniformTexCube = bgfx::createUniform("s_texCube", bgfx::UniformType::Sampler);
	m_uniformTexCubeIrr = bgfx::createUniform("s_texCubeIrr", bgfx::UniformType::Sampler);

	bgfx::ShaderHandle vsh = Renderer::LoadShader("vs_PBR_skybox");
	bgfx::ShaderHandle fsh = Renderer::LoadShader("fs_PBR_skybox");
	m_programSky = bgfx::createProgram(vsh, fsh, true);

	m_lightProbeTex = Renderer::LoadTexture("bolonga_lod.dds");
	m_lightProbeTexIrr = Renderer::LoadTexture("bolonga_irr.dds");
	m_lightProbeEV100 = -1.5f;
}

void SkyRenderer::Render(float deltaTime)
{
	bgfx::touch(GetViewID());

	const bx::Vec3 at = { 0.0f, 0.0f,   0.0f };
	const bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };
	float view[16];
	bx::mtxLookAt(view, eye, at);

	float proj[16];
	bx::mtxProj(proj, 60.0f, float(m_viewWidth) / float(m_viewHeight), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
	bgfx::setViewTransform(0, view, proj);
	bgfx::setViewRect(GetViewID(), 0, 0, m_viewWidth, m_viewHeight);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, MakeRBGA(128, 0, 0, 255), 1.0f, 0);

	bgfx::setTexture(0, m_uniformTexCube, m_lightProbeTex);
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
	ScreenSpaceQuad(m_viewWidth, m_viewHeight, true);
	bgfx::submit(GetViewID(), m_programSky);
}

}