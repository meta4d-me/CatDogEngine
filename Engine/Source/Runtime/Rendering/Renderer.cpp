#include "Renderer.h"

#include "ECWorld/StaticMeshComponent.h"
#include "Rendering/RenderContext.h"
#include "Rendering/RenderTarget.h"
#include "Rendering/Resources/MeshResource.h"
#include "Rendering/Resources/ResourceContext.h"
#include "Rendering/Resources/ShaderResource.h"

#include <bgfx/bgfx.h>

namespace engine
{

Renderer::Renderer(uint16_t viewID, RenderTarget* pRenderTarget)
	: m_viewID(viewID)
	, m_pRenderTarget(pRenderTarget)
{
}

static RenderContext* m_pRenderContext = nullptr;

void Renderer::SetRenderContext(RenderContext* pRenderContext)
{
	m_pRenderContext = pRenderContext;
}

RenderContext* Renderer::GetRenderContext()
{
	return m_pRenderContext;
}

void Renderer::UpdateViewRenderTarget()
{
	if (m_pRenderTarget)
	{
		bgfx::setViewFrameBuffer(GetViewID(), *GetRenderTarget()->GetFrameBufferHandle());
		bgfx::setViewRect(GetViewID(), 0, 0, GetRenderTarget()->GetWidth(), GetRenderTarget()->GetHeight());
	}
	else
	{
		assert(m_pRenderContext);
		bgfx::setViewRect(GetViewID(), 0, 0, m_pRenderContext->GetBackBufferWidth(), m_pRenderContext->GetBackBufferHeight());
	}
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

void Renderer::ScreenSpaceQuad(const RenderTarget* pRenderTarget, bool _originBottomLeft, float _width, float _height)
{
	float _textureWidth;
	float _textureHeight;
	if (pRenderTarget)
	{
		_textureWidth = pRenderTarget->GetWidth();
		_textureHeight = pRenderTarget->GetHeight();
	}
	else
	{
		assert(m_pRenderContext);
		_textureWidth = m_pRenderContext->GetBackBufferWidth();
		_textureHeight = m_pRenderContext->GetBackBufferHeight();
	}

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

void Renderer::SubmitStaticMeshDrawCall(StaticMeshComponent* pMeshComponent, uint16_t viewID, uint16_t programHandle)
{
	const MeshResource* pMeshResource = pMeshComponent->GetMeshResource();
	assert(ResourceStatus::Ready == pMeshResource->GetStatus() || ResourceStatus::Optimized == pMeshResource->GetStatus());
	bgfx::setVertexBuffer(0, bgfx::VertexBufferHandle{ pMeshResource->GetVertexBufferHandle() }, pMeshComponent->GetStartVertex(), pMeshComponent->GetVertexCount());
	for (uint32_t indexBufferIndex = 0U, indexBufferCount = pMeshResource->GetIndexBufferCount(); indexBufferIndex < indexBufferCount; ++indexBufferIndex)
	{
		bgfx::setIndexBuffer(bgfx::IndexBufferHandle{ pMeshResource->GetIndexBufferHandle(indexBufferIndex) }, pMeshComponent->GetStartIndex(), pMeshComponent->GetIndexCount());

		GetRenderContext()->Submit(viewID, programHandle);
	}
}

void Renderer::SubmitStaticMeshDrawCall(StaticMeshComponent* pMeshComponent, uint16_t viewID, StringCrc programHandleIndex)
{
	SubmitStaticMeshDrawCall(pMeshComponent, viewID, m_pRenderContext->GetResourceContext()->GetShaderResource(programHandleIndex)->GetHandle());
}

}