#include "EditorRenderer.h"

#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"

#include <bx/math.h>
#include <imgui.h>
#include <misc/freetype/imgui_freetype.h>

namespace editor
{

void EditorRenderer::Init()
{
	ImGuiIO& imguiIO = ImGui::GetIO();
	assert(!imguiIO.BackendPlatformUserData && "Already initialized a platform backend!");

	const engine::SwapChain* pSwapChain = GetSwapChain();
	imguiIO.DisplaySize = ImVec2(static_cast<float>(pSwapChain->GetWidth()), static_cast<float>(pSwapChain->GetHeight()));
	imguiIO.DeltaTime = 1.0f / 60.0f;

	ImFontAtlas* pFontAtlas = imguiIO.Fonts;
	pFontAtlas->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
	pFontAtlas->FontBuilderFlags = 0;
	pFontAtlas->Build();

	uint8_t* fontAtlasData;
	int32_t fontAtlasWidth;
	int32_t fontAtlasHeight;
	pFontAtlas->GetTexDataAsRGBA32(&fontAtlasData, &fontAtlasWidth, &fontAtlasHeight);
	m_imguiFontTexture = bgfx::createTexture2D(static_cast<uint16_t>(fontAtlasWidth), static_cast<uint16_t>(fontAtlasHeight), false, 1,
		bgfx::TextureFormat::BGRA8, 0, bgfx::copy(fontAtlasData, fontAtlasWidth * fontAtlasHeight * 4));

	m_imguiVertexLayout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	m_pRenderContext->CreateUniform("s_tex", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateProgram("ImGui", "vs_imgui.bin", "fs_imgui.bin");
}

EditorRenderer::~EditorRenderer()
{
}

void EditorRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	ImGui::Render();

	bgfx::setViewName(GetViewID(), "ImGui");
	bgfx::setViewMode(GetViewID(), bgfx::ViewMode::Sequential);
	bgfx::setViewFrameBuffer(GetViewID(), *GetSwapChain()->GetFrameBuffer());

	float ortho[16];
	const ImDrawData* pImGuiDrawData = ImGui::GetDrawData();
	float x = pImGuiDrawData->DisplayPos.x;
	float y = pImGuiDrawData->DisplayPos.y;
	float width = pImGuiDrawData->DisplaySize.x;
	float height = pImGuiDrawData->DisplaySize.y;
	const bgfx::Caps* pCapabilities = bgfx::getCaps();
	bx::mtxOrtho(ortho, x, x + width, y + height, y, 0.0f, 1000.0f, 0.0f, pCapabilities ? pCapabilities->homogeneousDepth : true);
	bgfx::setViewRect(GetViewID(), 0, 0, uint16_t(width), uint16_t(height));
	bgfx::setViewTransform(GetViewID(), nullptr, ortho);
	bgfx::setViewClear(GetViewID(), BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
}

void EditorRenderer::Render(float deltaTime)
{
	ImDrawData* pImGuiDrawData = ImGui::GetDrawData();

	int frameBufferWidth = static_cast<int>(pImGuiDrawData->DisplaySize.x * pImGuiDrawData->FramebufferScale.x);
	int frameBufferHeight = static_cast<int>(pImGuiDrawData->DisplaySize.y * pImGuiDrawData->FramebufferScale.y);
	const ImVec2 clipPos = pImGuiDrawData->DisplayPos;			// (0,0) unless using multi-viewports
	const ImVec2 clipScale = pImGuiDrawData->FramebufferScale;  // (1,1) unless using retina display which are often (2,2)

	for (int32_t commandListIndex = 0, numCommandLists = pImGuiDrawData->CmdListsCount; commandListIndex < numCommandLists; ++commandListIndex)
	{
		const ImDrawList* pDrawList = pImGuiDrawData->CmdLists[commandListIndex];

		uint32_t numVertices = static_cast<uint32_t>(pDrawList->VtxBuffer.size());
		uint32_t numIndices = static_cast<uint32_t>(pDrawList->IdxBuffer.size());
		const bool vertexBufferAvaiable = (numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, m_imguiVertexLayout));
		const bool indexBufferAvaiable = (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices));
		if (!vertexBufferAvaiable || !indexBufferAvaiable)
		{
			// not enough space in transient buffer just quit drawing the rest...
			break;
		}

		bgfx::TransientVertexBuffer vertexBuffer;
		bgfx::TransientIndexBuffer indexBuffer;
		bgfx::allocTransientVertexBuffer(&vertexBuffer, numVertices, m_imguiVertexLayout);
		bgfx::allocTransientIndexBuffer(&indexBuffer, numIndices, std::is_same<uint32_t, ImDrawIdx>());

		ImDrawVert* pVertices = reinterpret_cast<ImDrawVert*>(vertexBuffer.data);
		bx::memCopy(pVertices, pDrawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

		ImDrawIdx* pIndices = reinterpret_cast<ImDrawIdx*>(indexBuffer.data);
		bx::memCopy(pIndices, pDrawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));

		bgfx::Encoder* pEncoder = bgfx::begin();

		for (const ImDrawCmd* cmd = pDrawList->CmdBuffer.begin(), *cmdEnd = pDrawList->CmdBuffer.end(); cmd != cmdEnd; ++cmd)
		{
			if (cmd->UserCallback)
			{
				cmd->UserCallback(pDrawList, cmd);
			}
			else if (0 != cmd->ElemCount)
			{
				uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;
				bgfx::TextureHandle textureHandle = m_imguiFontTexture;
				if (nullptr != cmd->TextureId)
				{
					union
					{
						ImTextureID ptr;
						struct
						{
							bgfx::TextureHandle handle;
							uint8_t flags;
							uint8_t mip;
						} s;
					} texture = { cmd->TextureId };
					textureHandle = texture.s.handle;
				}
				else
				{
					state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
				}

				// Project scissor/clipping rectangles into framebuffer space
				ImVec4 clipRect;
				clipRect.x = (cmd->ClipRect.x - clipPos.x) * clipScale.x;
				clipRect.y = (cmd->ClipRect.y - clipPos.y) * clipScale.y;
				clipRect.z = (cmd->ClipRect.z - clipPos.x) * clipScale.x;
				clipRect.w = (cmd->ClipRect.w - clipPos.y) * clipScale.y;

				if (clipRect.x < frameBufferWidth &&
					clipRect.y < frameBufferHeight &&
					clipRect.z >= 0.0f &&
					clipRect.w >= 0.0f)
				{
					const uint16_t xx = static_cast<uint16_t>(bx::max(clipRect.x, 0.0f));
					const uint16_t yy = static_cast<uint16_t>(bx::max(clipRect.y, 0.0f));
					pEncoder->setScissor(xx, yy, uint16_t(bx::min(clipRect.z, 65535.0f) - xx), uint16_t(bx::min(clipRect.w, 65535.0f) - yy));

					pEncoder->setState(state);
					pEncoder->setTexture(0, m_pRenderContext->GetUniform(engine::StringCrc("s_tex")), textureHandle);
					pEncoder->setVertexBuffer(0, &vertexBuffer, cmd->VtxOffset, numVertices);
					pEncoder->setIndexBuffer(&indexBuffer, cmd->IdxOffset, cmd->ElemCount);
					pEncoder->submit(GetViewID(), m_pRenderContext->GetProgram(engine::StringCrc("ImGui")));
				}
			}
		}

		bgfx::end(pEncoder);
	}
}

}