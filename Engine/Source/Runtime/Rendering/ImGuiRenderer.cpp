#include "ImGuiRenderer.h"

#include "Rendering/RenderContext.h"
#include "Rendering/SwapChain.h"

#include <bx/math.h>
#include <imgui/imgui.h>
#include <misc/freetype/imgui_freetype.h>

namespace engine
{

void ImGuiRenderer::Init()
{
	ImGuiIO& imguiIO = ImGui::GetIO();
	assert(!imguiIO.BackendPlatformUserData && "Already initialized a platform backend!");
	imguiIO.DisplaySize = ImVec2(static_cast<float>(GetSwapChain()->GetWidth()), static_cast<float>(GetSwapChain()->GetHeight()));
	imguiIO.DeltaTime = 1.0f / 60;

	ImFontAtlas* pFontAtlas = imguiIO.Fonts;
	pFontAtlas->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
	pFontAtlas->FontBuilderFlags = 0;
	pFontAtlas->Build();

	uint8_t* pFontAtlasData;
	int32_t fontAtlasWidth;
	int32_t fontAtlasHeight;
	pFontAtlas->GetTexDataAsRGBA32(&pFontAtlasData, &fontAtlasWidth, &fontAtlasHeight);
	bgfx::TextureHandle imguiFontTexture = bgfx::createTexture2D(static_cast<uint16_t>(fontAtlasWidth), static_cast<uint16_t>(fontAtlasHeight), false, 1,
		bgfx::TextureFormat::BGRA8, 0, bgfx::copy(pFontAtlasData, fontAtlasWidth * fontAtlasHeight * 4));
	bgfx::setName(imguiFontTexture, "font_atlas");

	constexpr StringCrc fontAtlasTexture("font_atlas");
	m_pRenderContext->SetTexture(fontAtlasTexture, imguiFontTexture);

	bgfx::VertexLayout imguiVertexLayout;
	imguiVertexLayout.begin()
		.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	constexpr StringCrc imguiVertexLayoutName("imgui_vertex_layout");
	m_pRenderContext->SetVertexLayout(imguiVertexLayoutName, imguiVertexLayout);

	m_pRenderContext->CreateUniform("s_tex", bgfx::UniformType::Sampler);
	m_pRenderContext->CreateProgram("ImGuiProgram", "vs_imgui.bin", "fs_imgui.bin");
}

ImGuiRenderer::~ImGuiRenderer()
{
}

void ImGuiRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	ImGui::Render();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

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

void ImGuiRenderer::Render(float deltaTime)
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
		constexpr StringCrc imguiVertexLayoutName("imgui_vertex_layout");
		const bool vertexBufferAvaiable = (numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, m_pRenderContext->GetVertexLayout(imguiVertexLayoutName)));
		const bool indexBufferAvaiable = (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices));
		if (!vertexBufferAvaiable || !indexBufferAvaiable)
		{
			// not enough space in transient buffer just quit drawing the rest...
			break;
		}

		bgfx::TransientVertexBuffer vertexBuffer;
		bgfx::TransientIndexBuffer indexBuffer;
		bgfx::allocTransientVertexBuffer(&vertexBuffer, numVertices, m_pRenderContext->GetVertexLayout(imguiVertexLayoutName));
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
				bgfx::TextureHandle textureHandle;
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
					constexpr StringCrc fontAtlasTexture("font_atlas");
					textureHandle = m_pRenderContext->GetTexture(fontAtlasTexture);
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

					constexpr StringCrc textureSampler("s_tex");
					pEncoder->setTexture(0, m_pRenderContext->GetUniform(textureSampler), textureHandle);

					pEncoder->setVertexBuffer(0, &vertexBuffer, cmd->VtxOffset, numVertices);
					pEncoder->setIndexBuffer(&indexBuffer, cmd->IdxOffset, cmd->ElemCount);

					constexpr StringCrc imguiProgram("ImGuiProgram");
					pEncoder->submit(GetViewID(), m_pRenderContext->GetProgram(imguiProgram));
				}
			}
		}

		bgfx::end(pEncoder);
	}
}

}