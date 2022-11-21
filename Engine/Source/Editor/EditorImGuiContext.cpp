#include "EditorImGuiContext.h"

#include "IEditorImGuiLayer.h"

#include <bgfx/bgfx.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h> // used to customize style colors

#include <string>

namespace editor
{

EditorImGuiContext::EditorImGuiContext()
{
	m_pImGuiContext = ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	SetImGuiStyles();
}

EditorImGuiContext::~EditorImGuiContext()
{
	ImGui::DestroyContext(m_pImGuiContext);
}

void EditorImGuiContext::AddLayer(std::unique_ptr<IEditorImGuiLayer> pLayer)
{
	m_pImGuiLayers.emplace_back(std::move(pLayer));
}

void EditorImGuiContext::SetImGuiStyles()
{
	ImGuiStyle& style = ImGui::GetStyle();
	style.FramePadding.y = 0;
	style.ItemSpacing.y = 2;
	style.ItemInnerSpacing.x = 2;

	// When viewports are enabled we tweak WindowRounding/WindowBg
	// so platform windows can look identical to regular ones.
	if (ImGuiIO& io = ImGui::GetIO(); io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
	}

	// Adjust colors as you like after loading ImGui default style.
	ImGui::StyleColorsDark();
	ImVec4* colors = style.Colors;

	// I picked these color values from Visual Studio 2022.
	// Backgroud
	colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

	// Title
	colors[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);

	// Border
	//colors[ImGuiCol_Border] = ImVec4(0.44f, 0.38f, 0.91f, 1.0f);
	//colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);

	// Text
	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

	// Button
	colors[ImGuiCol_Button] = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);

	// ScolorBar
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.0f);

	// Slider
	colors[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);

	// Tab
	colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	colors[ImGuiCol_TabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);

	// Frame
	colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.0f);
}

void EditorImGuiContext::Update()
{
	ImGui::NewFrame();

	ImGui::ShowDemoWindow();
	//ImGui::ShowStyleEditor();

	//const ImGuiViewport* pViewport = ImGui::GetMainViewport();
	//ImGui::SetNextWindowPos(pViewport->WorkPos);
	//ImGui::SetNextWindowSize(pViewport->WorkSize);
	//ImGui::SetNextWindowViewport(pViewport->ID);
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	//ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar;
	//windowFlags |= ImGuiWindowFlags_NoDocking;
	//windowFlags |= ImGuiWindowFlags_NoTitleBar;
	//windowFlags |= ImGuiWindowFlags_NoCollapse;
	//windowFlags |= ImGuiWindowFlags_NoResize;
	//windowFlags |= ImGuiWindowFlags_NoMove;
	//windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
	//windowFlags |= ImGuiWindowFlags_NoNavFocus;
	//if (ImGui::Begin("ImGuiMainWindow", nullptr, windowFlags))
	//{
	//	ImGui::PopStyleVar(3);
	//
	//	ImGui::DockSpace(ImGui::GetID("MyDockSpace"));
	//
	//	for (const auto& pImGuiLayer : m_pImGuiLayers)
	//	{
	//		pImGuiLayer->Update();
	//	}
	//
	//	ImGui::End();
	//}
}

void EditorImGuiContext::LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language)
{
	ImGuiIO& io = ImGui::GetIO();

	ImFontConfig config;
	config.OversampleH = 2;
	config.OversampleV = 1;
	config.GlyphExtraSpacing.x = 1.0f;

	bool ttfFileLoaded = false;
	for (const std::string& ttfFileName : ttfFileNames)
	{
		std::string editorFontResourcePath = CDEDITOR_RESOURCES_ROOT_PATH;
		editorFontResourcePath += ttfFileName;

		const ImWchar* pGlyphRanges = nullptr;
		switch (language)
		{
		case engine::Language::ChineseSimplied:
			pGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
			break;
		case engine::Language::ChineseTraditional:
			pGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
			break;
		case engine::Language::Cyrillic:
			pGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
			break;
		case engine::Language::Greek:
			pGlyphRanges = io.Fonts->GetGlyphRangesGreek();
			break;
		case engine::Language::Japanese:
			pGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
			break;
		case engine::Language::Korean:
			pGlyphRanges = io.Fonts->GetGlyphRangesKorean();
			break;
		case engine::Language::Thai:
			pGlyphRanges = io.Fonts->GetGlyphRangesThai();
			break;
		case engine::Language::Vitnam:
			pGlyphRanges = io.Fonts->GetGlyphRangesVietnamese();
			break;
		case engine::Language::English:
		default:
			break;
		}

		if (ImFont* pFont = io.Fonts->AddFontFromFileTTF(editorFontResourcePath.c_str(), 15.0f, &config, pGlyphRanges))
		{
			ttfFileLoaded = true;
		}
	}

	// If no .ttf files loaded, we should use the default font.
	if (!ttfFileLoaded)
	{
		io.Fonts->AddFontDefault();
	}
}

void EditorImGuiContext::OnMouseLBDown()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left, true);
}

void EditorImGuiContext::OnMouseLBUp()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left, false);
}

void EditorImGuiContext::OnMouseRBDown()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Right, true);
}

void EditorImGuiContext::OnMouseRBUp()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Right, false);
}

void EditorImGuiContext::OnMouseMBDown()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Middle, true);
}

void EditorImGuiContext::OnMouseMBUp()
{
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
}

void EditorImGuiContext::OnMouseWheel(float offset)
{
	ImGui::GetIO().AddMouseWheelEvent(0.0f, offset);
}

void EditorImGuiContext::OnMouseMove(int32_t x, int32_t y)
{
	ImGui::GetIO().AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
}

}