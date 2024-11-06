#include "ImGuiRenderer.h"

#include "Core/StringCrc.h"
#include "Rendering/RenderContext.h"
#include "Rendering/Resources/ResourceContext.h"
#include "Rendering/Resources/ShaderResource.h"

#include <imgui/imgui.h>

namespace engine
{

void ImGuiRenderer::Init()
{
	AddDependentShaderResource(GetRenderContext()->RegisterShaderProgram("ImGuiProgram", "vs_imgui", "fs_imgui"));

	constexpr StringCrc imguiVertexLayoutName("imgui_vertex_layout");
	if (0 == GetRenderContext()->GetVertexAttributeLayouts(imguiVertexLayoutName).m_stride)
	{
		bgfx::VertexLayout imguiVertexLayout;
		imguiVertexLayout.begin()
			.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.end();
		GetRenderContext()->SetVertexLayout(imguiVertexLayoutName, std::move(imguiVertexLayout));
	}

	GetRenderContext()->CreateUniform("s_tex", bgfx::UniformType::Sampler);

	bgfx::setViewName(GetViewID(), "ImGuiRenderer");
}

void ImGuiRenderer::UpdateView(const float* pViewMatrix, const float* pProjectionMatrix)
{
	// ImGuiRenderer will be used by different ImGuiContext which has different font settings.
	// Shares the same font atlas now to save memory.
	// TODO : support different fonts in different ImGuiContext.
	constexpr StringCrc fontAtlasTexture("font_atlas");
	if (!bgfx::isValid(GetRenderContext()->GetTexture(fontAtlasTexture)))
	{
		ImFontAtlas* pFontAtlas = ImGui::GetIO().Fonts;
		assert(pFontAtlas->IsBuilt() && "The ImGui font atlas should be already built successfully.");
		uint8_t* pFontAtlasData;
		int32_t fontAtlasWidth;
		int32_t fontAtlasHeight;
		pFontAtlas->GetTexDataAsRGBA32(&pFontAtlasData, &fontAtlasWidth, &fontAtlasHeight);
		bgfx::TextureHandle imguiFontTexture = bgfx::createTexture2D(static_cast<uint16_t>(fontAtlasWidth), static_cast<uint16_t>(fontAtlasHeight), false, 1,
			bgfx::TextureFormat::BGRA8, 0, bgfx::copy(pFontAtlasData, fontAtlasWidth * fontAtlasHeight * 4));
		bgfx::setName(imguiFontTexture, "font_atlas");

		GetRenderContext()->SetTexture(fontAtlasTexture, std::move(imguiFontTexture));
	}

	ImGui::Render();
	ImGui::UpdatePlatformWindows();
	ImGui::RenderPlatformWindowsDefault();

	bgfx::setViewMode(GetViewID(), bgfx::ViewMode::Sequential);
	if (const engine::RenderTarget* pRenderTarget = GetRenderTarget())
	{
		bgfx::setViewFrameBuffer(GetViewID(), *(pRenderTarget->GetFrameBufferHandle()));
	}

	const ImDrawData* pImGuiDrawData = ImGui::GetDrawData();
	float x = pImGuiDrawData->DisplayPos.x;
	float y = pImGuiDrawData->DisplayPos.y;
	float width = pImGuiDrawData->DisplaySize.x;
	float height = pImGuiDrawData->DisplaySize.y;
	const bgfx::Caps* pCapabilities = bgfx::getCaps();

	cd::Matrix4x4 orthoMatrix = cd::Matrix4x4::Orthographic(x, x + width, y, y + height, 0.0f, 1000.0f, 0.0f, pCapabilities->homogeneousDepth);
	bgfx::setViewRect(GetViewID(), 0, 0, uint16_t(width), uint16_t(height));
	bgfx::setViewTransform(GetViewID(), nullptr, orthoMatrix.begin());
}

void ImGuiRenderer::Render(float deltaTime)
{
	for (const auto pResource : m_dependentShaderResources)
	{
		if (ResourceStatus::Ready != pResource->GetStatus() &&
			ResourceStatus::Optimized != pResource->GetStatus())
		{
			return;
		}
	}

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
		const bool vertexBufferAvaiable = (numVertices == bgfx::getAvailTransientVertexBuffer(numVertices, GetRenderContext()->GetVertexAttributeLayouts(imguiVertexLayoutName)));
		const bool indexBufferAvaiable = (0 == numIndices || numIndices == bgfx::getAvailTransientIndexBuffer(numIndices));
		if (!vertexBufferAvaiable || !indexBufferAvaiable)
		{
			// not enough space in transient buffer just quit drawing the rest...
			break;
		}

		bgfx::TransientVertexBuffer vertexBuffer;
		bgfx::TransientIndexBuffer indexBuffer;
		bgfx::allocTransientVertexBuffer(&vertexBuffer, numVertices, GetRenderContext()->GetVertexAttributeLayouts(imguiVertexLayoutName));
		bgfx::allocTransientIndexBuffer(&indexBuffer, numIndices, std::is_same<uint32_t, ImDrawIdx>());

		ImDrawVert* pVertices = reinterpret_cast<ImDrawVert*>(vertexBuffer.data);
		std::memcpy(pVertices, pDrawList->VtxBuffer.begin(), numVertices * sizeof(ImDrawVert));

		ImDrawIdx* pIndices = reinterpret_cast<ImDrawIdx*>(indexBuffer.data);
		std::memcpy(pIndices, pDrawList->IdxBuffer.begin(), numIndices * sizeof(ImDrawIdx));

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
							uint16_t unused;
						} s;
					} texture = { cmd->TextureId };
					textureHandle = texture.s.handle;

					// Some widgets such as color picker need to use color blend.
					// TODO : only open color blend for color picker?
					state |= BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
				}
				else
				{
					constexpr StringCrc fontAtlasTexture("font_atlas");
					textureHandle = GetRenderContext()->GetTexture(fontAtlasTexture);
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
					const uint16_t xx = static_cast<uint16_t>(std::max(clipRect.x, 0.0f));
					const uint16_t yy = static_cast<uint16_t>(std::max(clipRect.y, 0.0f));
					pEncoder->setScissor(xx, yy, uint16_t(std::min(clipRect.z, 65535.0f) - xx), uint16_t(std::min(clipRect.w, 65535.0f) - yy));

					pEncoder->setState(state);

					constexpr StringCrc textureSampler("s_tex");
					pEncoder->setTexture(0, GetRenderContext()->GetUniform(textureSampler), textureHandle);

					pEncoder->setVertexBuffer(0, &vertexBuffer, cmd->VtxOffset, numVertices);
					pEncoder->setIndexBuffer(&indexBuffer, cmd->IdxOffset, cmd->ElemCount);

					constexpr StringCrc programHandleIndex{ "ImGuiProgram" };
					pEncoder->submit(GetViewID(), bgfx::ProgramHandle{ GetRenderContext()->GetResourceContext()->GetShaderResource(programHandleIndex)->GetHandle()});
				}
			}
		}

		bgfx::end(pEncoder);
	}
}

}