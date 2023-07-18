#include "Splash.h"

#include "Rendering/RenderContext.h"
#include "Resources/ResourceBuilder.h"

#include <imgui/imgui.h>

//#include <format>

namespace editor
{

Splash::~Splash()
{

}

void Splash::Init()
{
	GetRenderContext()->CreateTexture("Textures/lut/ibl_brdf_lut.dds");

	m_shaderBuildTaskTotalCount = ResourceBuilder::Get().GetCurrentTaskCount();
}

void Splash::Update()
{
	auto flags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	
	size_t currentBuildCount = ResourceBuilder::Get().GetCurrentTaskCount();
	assert(currentBuildCount <= m_shaderBuildTaskTotalCount);

	//std::string title = std::format("{}({}/{})", GetName(), m_shaderBuildTaskTotalCount - currentBuildCount, m_shaderBuildTaskTotalCount);
	std::string title = GetName();
	title += "(" + std::to_string(m_shaderBuildTaskTotalCount - currentBuildCount) + "/" + std::to_string(m_shaderBuildTaskTotalCount) + ")";
	ImGui::Begin(title.c_str(), &m_isEnable, flags);

	engine::StringCrc splashTexture("Textures/lut/ibl_brdf_lut.dds");
	bgfx::TextureHandle textureHandle = GetRenderContext()->GetTexture(splashTexture);
	ImGui::Image(ImTextureID(textureHandle.idx), ImVec2(500, 300));

	ImGui::End();
}

}