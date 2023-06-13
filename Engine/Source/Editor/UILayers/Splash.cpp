#include "Splash.h"

#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"

#include <imgui/imgui.h>

#include <format>

namespace editor
{

Splash::~Splash()
{

}

void Splash::Init()
{
	GetRenderContext()->CreateTexture("Textures/image.png");

	m_shaderBuildTaskTotalCount = ResourceBuilder::Get().GetCurrentTaskCount();
}

void Splash::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar;

	size_t currentBuildCount = ResourceBuilder::Get().GetCurrentTaskCount();
	assert(currentBuildCount <= m_shaderBuildTaskTotalCount);

	std::string title = std::format("{}({}/{})", GetName(), m_shaderBuildTaskTotalCount - currentBuildCount, m_shaderBuildTaskTotalCount);
	ImGui::Begin(" ", &m_isEnable, flags);

	engine::StringCrc splashTexture("Textures/image.png");
	bgfx::TextureHandle textureHandle = GetRenderContext()->GetTexture(splashTexture);
	ImGui::Image(ImTextureID(textureHandle.idx), ImVec2(400, 300));
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Text("%s", title.c_str());
	ImGui::Separator();	
	ImGui::End();
}

}