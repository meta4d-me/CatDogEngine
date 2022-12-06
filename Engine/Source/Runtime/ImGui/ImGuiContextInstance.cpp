#include "ImGuiContextInstance.h"

#include "IconFont/IconsMaterialDesignIcons.h"
#include "IconFont/MaterialDesign.inl"
#include "ImGui/ImGuiBaseLayer.h"

#include <bgfx/bgfx.h>
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <misc/freetype/imgui_freetype.h>

#include <string>

namespace
{

class TempSwitchContextScope
{
public:
	TempSwitchContextScope(engine::ImGuiContextInstance* pThis)
	{
		ImGuiIO& io = ImGui::GetIO();
		assert(io.UserData != nullptr && "Please set ImGuiContextInstance to io.UserData field.");
		if (io.UserData != pThis)
		{
			pThis->SwitchCurrentContext();
			pBackContext = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
		}
	}

	TempSwitchContextScope(const TempSwitchContextScope&) = delete;
	TempSwitchContextScope& operator=(const TempSwitchContextScope&) = delete;
	TempSwitchContextScope(TempSwitchContextScope&&) = delete;
	TempSwitchContextScope& operator=(TempSwitchContextScope&&) = delete;

	~TempSwitchContextScope()
	{
		if (pBackContext)
		{
			pBackContext->SwitchCurrentContext();
		}
	}

private:
	engine::ImGuiContextInstance* pBackContext = nullptr;
};

}

namespace engine
{

ImGuiContextInstance::ImGuiContextInstance(uint16_t width, uint16_t height)
{
	m_pImGuiContext = ImGui::CreateContext();
	SwitchCurrentContext();

	ImGuiIO& io = ImGui::GetIO();
	// It will be very useful for UI layers to get/set data in the current ImGuiContext.
	io.UserData = static_cast<void*>(this);
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	// Avoid moving window too easy to trigger.
	io.ConfigWindowsMoveFromTitleBarOnly = true;

	// TODO
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	io.IniFilename = nullptr;
	io.LogFilename = nullptr;

	// TODO : dynamic resize or modify fps.
	io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
	io.DeltaTime = 1.0f / 60;

	SetImGuiStyles();
}

ImGuiContextInstance::~ImGuiContextInstance()
{
	ImGui::DestroyContext(m_pImGuiContext);
}

void ImGuiContextInstance::SwitchCurrentContext() const
{
	ImGui::SetCurrentContext(m_pImGuiContext);
}

void ImGuiContextInstance::AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer)
{
	pLayer->Init();
	m_pImGuiStaticLayers.emplace_back(std::move(pLayer));
}

void ImGuiContextInstance::AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer)
{
	pLayer->Init();
	m_pImGuiDockableLayers.emplace_back(std::move(pLayer));
}

void ImGuiContextInstance::BeginDockSpace()
{
	// To create a dock space, we need to create a window to host it at first.
	constexpr const char* pDockSpaceName = "FullScreenDockSpace";
	static bool enableDockSpace = true;
	constexpr ImGuiWindowFlags dockSpaceWindowFlags = ImGuiWindowFlags_NoDocking |
													  ImGuiWindowFlags_NoTitleBar |
													  ImGuiWindowFlags_NoCollapse |
													  ImGuiWindowFlags_NoResize |
													  ImGuiWindowFlags_NoMove |
													  ImGuiWindowFlags_NoBringToFrontOnFocus |
													  ImGuiWindowFlags_NoNavFocus |
													  ImGuiWindowFlags_NoBackground;

	// Place dock space window under static imgui layers.
	// It is a hack now as only main menu bar is a static layer so we only need to adjust the height.
	// If we have more static layers, we need to calculate their accurate areas.
	ImGuiViewport* pMainViewport = ImGui::GetMainViewport();
	assert(pMainViewport && "The main viewport cannot be null");
	const float mainMenuBarSize = ImGui::GetFrameHeight();
	ImVec2 dockSpacePos = pMainViewport->Pos;
	dockSpacePos.y += mainMenuBarSize;
	ImVec2 dockSpaceSize = pMainViewport->Size;
	dockSpaceSize.y -= mainMenuBarSize;
	ImGui::SetNextWindowPos(dockSpacePos);
	ImGui::SetNextWindowSize(dockSpaceSize);
	ImGui::SetNextWindowViewport(pMainViewport->ID);

	// Create the dock space host window.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin(pDockSpaceName, &enableDockSpace, dockSpaceWindowFlags);
	ImGui::PopStyleVar(3);

	// Build other child dock spaces in the window.
	ImGuiID dockSpaceWindowID = ImGui::GetID(pDockSpaceName);
	if (!ImGui::DockBuilderGetNode(dockSpaceWindowID))
	{
		ImGui::DockBuilderRemoveNode(dockSpaceWindowID);
		ImGui::DockBuilderAddNode(dockSpaceWindowID);
		ImGui::DockBuilderSetNodeSize(dockSpaceWindowID, ImGui::GetIO().DisplaySize * ImGui::GetIO().DisplayFramebufferScale);

		ImGuiID dockSpaceMainID = dockSpaceWindowID;

		ImGuiID dockSpaceUp = ImGui::DockBuilderSplitNode(dockSpaceMainID, ImGuiDir_Up, 0.7f, nullptr, &dockSpaceMainID);
		ImGuiID dockSpaceBottom = ImGui::DockBuilderSplitNode(dockSpaceMainID, ImGuiDir_Down, 0.3f, nullptr, &dockSpaceMainID);

		ImGuiID dockSpaceUpLeft = ImGui::DockBuilderSplitNode(dockSpaceUp, ImGuiDir_Left, 0.7f, nullptr, &dockSpaceUp);
		ImGuiID dockSpaceUpLeftLeft = ImGui::DockBuilderSplitNode(dockSpaceUpLeft, ImGuiDir_Left, 0.3f, nullptr, &dockSpaceUpLeft);
		ImGuiID dockSpaceUpRight = ImGui::DockBuilderSplitNode(dockSpaceUp, ImGuiDir_Right, 0.3f, nullptr, &dockSpaceUp);

		ImGuiID dockSpaceLeftLeft = ImGui::DockBuilderSplitNode(dockSpaceUpLeft, ImGuiDir_Left, 0.5f, nullptr, &dockSpaceUpLeft);
		ImGuiID dockSpaceLeftRight = ImGui::DockBuilderSplitNode(dockSpaceUpLeft, ImGuiDir_Right, 0.5f, nullptr, &dockSpaceUpLeft);

		ImGuiID dockSpaceBottomLeft = ImGui::DockBuilderSplitNode(dockSpaceBottom, ImGuiDir_Left, 0.5f, nullptr, &dockSpaceBottom);
		ImGuiID dockSpaceBottomRight = ImGui::DockBuilderSplitNode(dockSpaceBottom, ImGuiDir_Right, 0.5f, nullptr, &dockSpaceBottom);

		// Register dockable layers.
		ImGui::DockBuilderDockWindow("EntityList", dockSpaceUpLeftLeft);
		ImGui::DockBuilderDockWindow("GameView", dockSpaceLeftLeft);
		ImGui::DockBuilderDockWindow("SceneView", dockSpaceLeftRight);
		ImGui::DockBuilderDockWindow("Inspector", dockSpaceUpRight);
		ImGui::DockBuilderDockWindow("AssetBrowser", dockSpaceBottomLeft);
		ImGui::DockBuilderDockWindow("OutputLog", dockSpaceBottomRight);

		ImGui::DockBuilderFinish(dockSpaceWindowID);
	}

	// Create a dock space by ImGui default settings by using the created host window.
	ImGui::DockSpace(dockSpaceWindowID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton);
}

void ImGuiContextInstance::EndDockSpace()
{
	ImGui::End();
}

void ImGuiContextInstance::Update()
{
	ImGui::NewFrame();
	
	for (const auto& pImGuiLayer : m_pImGuiStaticLayers)
	{
		pImGuiLayer->Update();
	}

	BeginDockSpace();

	for (const auto& pImGuiLayer : m_pImGuiDockableLayers)
	{
		if (pImGuiLayer->IsEnable())
		{
			pImGuiLayer->Update();
		}
	}

	EndDockSpace();

	//ImGui::ShowDemoWindow();
	//ImGui::ShowStyleEditor();
}

void ImGuiContextInstance::OnResize(uint16_t width, uint16_t height)
{
	// It is a callback method which can happen in another ImGuiContext.
	// So we hope it switch back after finishing jobs.
	TempSwitchContextScope tempSwitchScope(this);

	ImGuiIO& io = ImGui::GetIO();
	io.DisplaySize.x = width;
	io.DisplaySize.y = height;
}

void ImGuiContextInstance::OnMouseLBDown()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left, true);
}

void ImGuiContextInstance::OnMouseLBUp()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Left, false);
}

void ImGuiContextInstance::OnMouseRBDown()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Right, true);
}

void ImGuiContextInstance::OnMouseRBUp()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Right, false);
}

void ImGuiContextInstance::OnMouseMBDown()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Middle, true);
}

void ImGuiContextInstance::OnMouseMBUp()
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseButtonEvent(ImGuiMouseButton_Middle, false);
}

void ImGuiContextInstance::OnMouseWheel(float offset)
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMouseWheelEvent(0.0f, offset);
}

void ImGuiContextInstance::OnMouseMove(int32_t x, int32_t y)
{
	TempSwitchContextScope tempSwitchScope(this);
	ImGui::GetIO().AddMousePosEvent(static_cast<float>(x), static_cast<float>(y));
}

void ImGuiContextInstance::LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language)
{
	TempSwitchContextScope tempSwitchScope(this);

	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0f;

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

	// IconFont
	ImFontConfig iconFontConfig;
	iconFontConfig.MergeMode = true;
	iconFontConfig.PixelSnapH = true;
	iconFontConfig.GlyphOffset.y = 1.0f;
	iconFontConfig.OversampleH = iconFontConfig.OversampleV = 1;
	iconFontConfig.GlyphMinAdvanceX = 4.0f;
	iconFontConfig.SizePixels = 12.0f;

	// MaterialDesignIconFont is from https://materialdesignicons.com/, then generated a c style header file to use in memory without loading from disk.
	// Note that font glyph range array needs to be persistent until you build the font. So it will be convenient to declare it as static.
	static ImWchar iconFontGlyphRange[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };
	io.Fonts->AddFontFromMemoryCompressedTTF(MaterialDesign_compressed_data, MaterialDesign_compressed_size, 14.0f, &iconFontConfig, iconFontGlyphRange);
	io.Fonts->TexGlyphPadding = 1;
	for (int fontConfigDataIndex = 0; fontConfigDataIndex < io.Fonts->ConfigData.Size; ++fontConfigDataIndex)
	{
		io.Fonts->ConfigData[fontConfigDataIndex].RasterizerMultiply = 1.0f;
	}

	ImFontAtlas* pFontAtlas = ImGui::GetIO().Fonts;
	pFontAtlas->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
	pFontAtlas->FontBuilderFlags = 0;
	pFontAtlas->Build();
}

void ImGuiContextInstance::SetImGuiStyles()
{
	TempSwitchContextScope tempSwitchScope(this);

	ImGuiStyle& style = ImGui::GetStyle();
	style.WindowPadding = ImVec2(5, 5);
	style.FramePadding = ImVec2(4, 4);
	style.ItemSpacing = ImVec2(6, 2);
	style.ItemInnerSpacing = ImVec2(2, 2);
	style.IndentSpacing = 6.0f;
	style.TouchExtraPadding = ImVec2(4, 4);

	style.ScrollbarSize = 10;

	style.WindowBorderSize = 0;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 3;
	style.FrameBorderSize = 0.0f;

	constexpr int roundingAmount = 2;
	style.PopupRounding = roundingAmount;
	style.WindowRounding = roundingAmount;
	style.ChildRounding = 0;
	style.FrameRounding = roundingAmount;
	style.ScrollbarRounding = roundingAmount;
	style.GrabRounding = roundingAmount;
	style.WindowMinSize = ImVec2(200.0f, 200.0f);

	style.TabBorderSize = 1.0f;
	style.TabRounding = roundingAmount;

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = roundingAmount;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

void ImGuiContextInstance::SetImGuiThemeColor(ThemeColor theme)
{
	TempSwitchContextScope tempSwitchScope(this);

	m_themeColor = theme;

	ImVec4* colours = ImGui::GetStyle().Colors;
	if (ThemeColor::Black == theme)
	{
		ImGui::StyleColorsDark();
		colours[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colours[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colours[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colours[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colours[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		colours[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		colours[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colours[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colours[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colours[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colours[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colours[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colours[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colours[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colours[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colours[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colours[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colours[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colours[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colours[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colours[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colours[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colours[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colours[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colours[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colours[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colours[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colours[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colours[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colours[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colours[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colours[ImGuiCol_DockingEmptyBg] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);;
		colours[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colours[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colours[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colours[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colours[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colours[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colours[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colours[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colours[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colours[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	}
	else if (ThemeColor::Classic == theme)
	{
		ImGui::StyleColorsClassic();
	}
	else if (ThemeColor::Dark == theme)
	{
		ImGui::StyleColorsDark();
		constexpr float max = 255.0f;
		ImVec4 Titlebar = ImVec4(40.0f / max, 42.0f / max, 54.0f / max, 1.0f);
		ImVec4 TabActive = ImVec4(52.0f / max, 54.0f / max, 64.0f / max, 1.0f);
		ImVec4 TabUnactive = ImVec4(35.0f / max, 43.0f / max, 59.0f / max, 1.0f);
		colours[ImGuiCol_Text] = ImVec4(200.0f / 255.0f, 200.0f / 255.0f, 200.0f / 255.0f, 1.00f);
		colours[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
		colours[ImGuiCol_WindowBg] = TabActive;
		colours[ImGuiCol_ChildBg] = TabActive;
		colours[ImGuiCol_PopupBg] = ImVec4(42.0f / 255.0f, 38.0f / 255.0f, 47.0f / 255.0f, 1.00f);
		colours[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
		colours[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colours[ImGuiCol_FrameBg] = ImVec4(65.0f / 255.0f, 79.0f / 255.0f, 92.0f / 255.0f, 1.00f);
		colours[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
		colours[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
		colours[ImGuiCol_TitleBg] = Titlebar;
		colours[ImGuiCol_TitleBgActive] = Titlebar;
		colours[ImGuiCol_TitleBgCollapsed] = Titlebar;
		colours[ImGuiCol_MenuBarBg] = Titlebar;
		colours[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
		colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.6f, 0.6f, 0.6f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.8f, 0.8f, 0.8f, 1.00f);
		colours[ImGuiCol_CheckMark] = ImVec4(155.0f / 255.0f, 130.0f / 255.0f, 207.0f / 255.0f, 1.00f);
		colours[ImGuiCol_SliderGrab] = ImVec4(155.0f / 255.0f, 130.0f / 255.0f, 207.0f / 255.0f, 1.00f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(185.0f / 255.0f, 160.0f / 255.0f, 237.0f / 255.0f, 1.00f);
		colours[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		colours[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f) + ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
		colours[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f) + ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
		colours[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
		colours[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
		colours[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
		colours[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colours[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colours[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colours[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colours[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colours[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
		colours[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colours[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colours[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colours[ImGuiCol_Header] = TabActive + ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
		colours[ImGuiCol_HeaderHovered] = TabActive + ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
		colours[ImGuiCol_HeaderActive] = TabActive + ImVec4(0.05f, 0.05f, 0.05f, 0.1f);
		colours[ImGuiCol_Tab] = TabUnactive;
		colours[ImGuiCol_TabHovered] = TabActive + ImVec4(0.1f, 0.1f, 0.1f, 0.1f);
		colours[ImGuiCol_TabActive] = TabActive;
		colours[ImGuiCol_TabUnfocused] = TabUnactive;
		colours[ImGuiCol_TabUnfocusedActive] = TabActive;
		colours[ImGuiCol_DockingEmptyBg] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colours[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	}
	else if (ThemeColor::Grey == theme)
	{
		ImGui::StyleColorsDark();
		colours[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colours[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colours[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colours[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colours[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colours[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
		colours[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colours[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
		colours[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
		colours[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
		colours[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
		colours[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
		colours[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
		colours[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
		colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colours[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
		colours[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
		colours[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
		colours[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
		colours[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
		colours[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colours[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
		colours[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
		colours[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
		colours[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
		colours[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
		colours[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colours[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colours[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
		colours[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colours[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colours[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
		colours[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
		colours[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colours[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colours[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colours[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
		colours[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colours[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
		colours[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colours[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
		colours[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
		colours[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);
	}
	else if (ThemeColor::Light == theme)
	{
		ImGui::StyleColorsLight();
		colours[ImGuiCol_Text] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colours[ImGuiCol_TextDisabled] = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
		colours[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.94f);
		colours[ImGuiCol_PopupBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		colours[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.39f);
		colours[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
		colours[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.94f);
		colours[ImGuiCol_FrameBgHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		colours[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colours[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 1.00f);
		colours[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 1.00f, 1.00f, 0.51f);
		colours[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		colours[ImGuiCol_MenuBarBg] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
		colours[ImGuiCol_ScrollbarBg] = ImVec4(0.98f, 0.98f, 0.98f, 0.53f);
		colours[ImGuiCol_ScrollbarGrab] = ImVec4(0.69f, 0.69f, 0.69f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 1.00f);
		colours[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colours[ImGuiCol_CheckMark] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_SliderGrab] = ImVec4(0.24f, 0.52f, 0.88f, 1.00f);
		colours[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_Button] = ImVec4(0.26f, 0.59f, 0.98f, 0.40f);
		colours[ImGuiCol_ButtonHovered] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
		colours[ImGuiCol_Header] = ImVec4(0.26f, 0.59f, 0.98f, 0.31f);
		colours[ImGuiCol_HeaderHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.80f);
		colours[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
		colours[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.50f);
		colours[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
		colours[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
		colours[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colours[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
		colours[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
		colours[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
		colours[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	}

	colours[ImGuiCol_Separator] = colours[ImGuiCol_TitleBg];
	colours[ImGuiCol_SeparatorActive] = colours[ImGuiCol_Separator];
	colours[ImGuiCol_SeparatorHovered] = colours[ImGuiCol_Separator];
	colours[ImGuiCol_Tab] = colours[ImGuiCol_MenuBarBg];
	colours[ImGuiCol_TabUnfocused] = colours[ImGuiCol_MenuBarBg];
	colours[ImGuiCol_TabUnfocusedActive] = colours[ImGuiCol_WindowBg];
	colours[ImGuiCol_TabActive] = colours[ImGuiCol_WindowBg];
	colours[ImGuiCol_ChildBg] = colours[ImGuiCol_TabActive];
	colours[ImGuiCol_ScrollbarBg] = colours[ImGuiCol_TabActive];
	colours[ImGuiCol_TitleBgActive] = colours[ImGuiCol_TitleBg];
	colours[ImGuiCol_TitleBgCollapsed] = colours[ImGuiCol_TitleBg];
	colours[ImGuiCol_MenuBarBg] = colours[ImGuiCol_TitleBg];
	colours[ImGuiCol_PopupBg] = colours[ImGuiCol_WindowBg] + ImVec4(0.05f, 0.05f, 0.05f, 0.0f);
	colours[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 0.00f);
	colours[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
}

}