#include "ImGuiContextInstance.h"

#include "IconFont/IconsMaterialDesignIcons.h"
#include "IconFont/MaterialDesign.inl"
#include "ImGui/ImGuiBaseLayer.h"
#include "Rendering/RenderContext.h"
#include "Rendering/RenderTarget.h"
#include "Window/Input.h"
#include "Window/Window.h"
#include "Window/WindowManager.h"
#include "Log/Log.h"

#include <bgfx/bgfx.h>
#include <imgui/imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#ifdef IMGUI_ENABLE_FREETYPE
#include <misc/freetype/imgui_freetype.h>
#endif

#include <format>
#include <string>
#include <unordered_map>

namespace
{

std::unordered_map<engine::KeyCode, ImGuiKey> kImguiKeyLookup {
	{engine::KeyCode::RETURN, ImGuiKey::ImGuiKey_Enter},
	{ engine::KeyCode::ESCAPE, ImGuiKey::ImGuiKey_Escape },
	{ engine::KeyCode::BACKSPACE, ImGuiKey::ImGuiKey_Backspace },
	{ engine::KeyCode::TAB, ImGuiKey::ImGuiKey_Tab },
	{ engine::KeyCode::SPACE, ImGuiKey::ImGuiKey_Space },
		// {engine::KeyCode::EXCLAIM, ImGuiKey::ImGuiKey_}, 
		// {engine::KeyCode::QUOTEDBL, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::HASH, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::PERCENT, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::DOLLAR, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::AMPERSAND, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::QUOTE, ImGuiKey::ImGuiKey_Apostrophe },
		// {engine::KeyCode::LEFTPAREN, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::RIGHTPAREN, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::ASTERISK, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::PLUS, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::COMMA, ImGuiKey::ImGuiKey_Comma },
	{ engine::KeyCode::MINUS, ImGuiKey::ImGuiKey_Minus },
	{ engine::KeyCode::PERIOD, ImGuiKey::ImGuiKey_Period },
	{ engine::KeyCode::SLASH, ImGuiKey::ImGuiKey_Slash },
	{ engine::KeyCode::NUM_0, ImGuiKey::ImGuiKey_0 },
	{ engine::KeyCode::NUM_1, ImGuiKey::ImGuiKey_1 },
	{ engine::KeyCode::NUM_2, ImGuiKey::ImGuiKey_2 },
	{ engine::KeyCode::NUM_3, ImGuiKey::ImGuiKey_3 },
	{ engine::KeyCode::NUM_4, ImGuiKey::ImGuiKey_4 },
	{ engine::KeyCode::NUM_5, ImGuiKey::ImGuiKey_5 },
	{ engine::KeyCode::NUM_6, ImGuiKey::ImGuiKey_6 },
	{ engine::KeyCode::NUM_7, ImGuiKey::ImGuiKey_7 },
	{ engine::KeyCode::NUM_8, ImGuiKey::ImGuiKey_8 },
	{ engine::KeyCode::NUM_9, ImGuiKey::ImGuiKey_9 },
		// {engine::KeyCode::COLON, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::SEMICOLON, ImGuiKey::ImGuiKey_Semicolon },
		// {engine::KeyCode::LESS, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::EQUALS, ImGuiKey::ImGuiKey_Equal },
		// {engine::KeyCode::GREATER, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::QUESTION, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::AT, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::LEFTBRACKET, ImGuiKey::ImGuiKey_LeftBracket },
	{ engine::KeyCode::BACKSLASH, ImGuiKey::ImGuiKey_Backslash },
	{ engine::KeyCode::RIGHTBRACKET, ImGuiKey::ImGuiKey_RightBracket },
		// {engine::KeyCode::CARET, ImGuiKey::ImGuiKey_},
		// {engine::KeyCode::UNDERSCORE, ImGuiKey::ImGuiKey_},
	{ engine::KeyCode::BACKQUOTE, ImGuiKey::ImGuiKey_GraveAccent },
	{ engine::KeyCode::a, ImGuiKey::ImGuiKey_A },
	{ engine::KeyCode::b, ImGuiKey::ImGuiKey_B },
	{ engine::KeyCode::c, ImGuiKey::ImGuiKey_C },
	{ engine::KeyCode::d, ImGuiKey::ImGuiKey_D },
	{ engine::KeyCode::e, ImGuiKey::ImGuiKey_E },
	{ engine::KeyCode::f, ImGuiKey::ImGuiKey_F },
	{ engine::KeyCode::g, ImGuiKey::ImGuiKey_G },
	{ engine::KeyCode::h, ImGuiKey::ImGuiKey_H },
	{ engine::KeyCode::i, ImGuiKey::ImGuiKey_I },
	{ engine::KeyCode::j, ImGuiKey::ImGuiKey_J },
	{ engine::KeyCode::k, ImGuiKey::ImGuiKey_K },
	{ engine::KeyCode::l, ImGuiKey::ImGuiKey_L },
	{ engine::KeyCode::m, ImGuiKey::ImGuiKey_M },
	{ engine::KeyCode::n, ImGuiKey::ImGuiKey_N },
	{ engine::KeyCode::o, ImGuiKey::ImGuiKey_O },
	{ engine::KeyCode::p, ImGuiKey::ImGuiKey_P },
	{ engine::KeyCode::q, ImGuiKey::ImGuiKey_Q },
	{ engine::KeyCode::r, ImGuiKey::ImGuiKey_R },
	{ engine::KeyCode::s, ImGuiKey::ImGuiKey_S },
	{ engine::KeyCode::t, ImGuiKey::ImGuiKey_T },
	{ engine::KeyCode::u, ImGuiKey::ImGuiKey_U },
	{ engine::KeyCode::v, ImGuiKey::ImGuiKey_V },
	{ engine::KeyCode::w, ImGuiKey::ImGuiKey_W },
	{ engine::KeyCode::x, ImGuiKey::ImGuiKey_X },
	{ engine::KeyCode::y, ImGuiKey::ImGuiKey_Y },
	{ engine::KeyCode::z, ImGuiKey::ImGuiKey_Z },
};

std::unordered_map<engine::KeyMod, ImGuiKey> kImguiKeyModToImGuiKeyLookup{
	{engine::KeyMod::KMOD_LSHIFT, ImGuiKey::ImGuiKey_LeftShift},
	{ engine::KeyMod::KMOD_RSHIFT, ImGuiKey::ImGuiKey_RightShift },
	{ engine::KeyMod::KMOD_LCTRL, ImGuiKey::ImGuiKey_LeftCtrl },
	{ engine::KeyMod::KMOD_RCTRL, ImGuiKey::ImGuiKey_RightCtrl },
	{ engine::KeyMod::KMOD_LALT, ImGuiKey::ImGuiKey_LeftAlt },
	{ engine::KeyMod::KMOD_RALT, ImGuiKey::ImGuiKey_RightAlt },
	{ engine::KeyMod::KMOD_LGUI, ImGuiKey::ImGuiKey_LeftSuper },
	{ engine::KeyMod::KMOD_RGUI, ImGuiKey::ImGuiKey_RightSuper },
	{ engine::KeyMod::KMOD_NUM, ImGuiKey::ImGuiKey_NumLock },
	{ engine::KeyMod::KMOD_CAPS, ImGuiKey::ImGuiKey_CapsLock },
	{ engine::KeyMod::KMOD_MODE, ImGuiKey::ImGuiKey_ModSuper },
	{ engine::KeyMod::KMOD_SCROLL, ImGuiKey::ImGuiKey_ScrollLock },
};

std::unordered_map<engine::KeyMod, ImGuiKey> kImguiKeyModToImGuiModLookup{
	{engine::KeyMod::KMOD_NONE, ImGuiKey::ImGuiMod_None},
	{ engine::KeyMod::KMOD_LSHIFT, ImGuiKey::ImGuiMod_Shift },
	{ engine::KeyMod::KMOD_RSHIFT, ImGuiKey::ImGuiMod_Shift },
	{ engine::KeyMod::KMOD_LCTRL, ImGuiKey::ImGuiMod_Ctrl },
	{ engine::KeyMod::KMOD_RCTRL, ImGuiKey::ImGuiMod_Ctrl },
	{ engine::KeyMod::KMOD_LALT, ImGuiKey::ImGuiMod_Alt },
	{ engine::KeyMod::KMOD_RALT, ImGuiKey::ImGuiMod_Alt },
	{ engine::KeyMod::KMOD_LGUI, ImGuiKey::ImGuiMod_Super },
	{ engine::KeyMod::KMOD_RGUI, ImGuiKey::ImGuiMod_Super },
	{ engine::KeyMod::KMOD_CTRL, ImGuiKey::ImGuiMod_Ctrl },
	{ engine::KeyMod::KMOD_SHIFT, ImGuiKey::ImGuiMod_Shift },
	{ engine::KeyMod::KMOD_ALT, ImGuiKey::ImGuiMod_Alt },
	{ engine::KeyMod::KMOD_GUI, ImGuiKey::ImGuiMod_Super },
};

}

namespace engine
{

ImGuiContextInstance::ImGuiContextInstance()
{
	m_pImGuiContext = ImGui::CreateContext();

	// Basic settings which can export as APIs if need.
	ImGuiIO& io = GetIO();
	io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
	io.UserData = this; // Help to find ImGuiContextInstance from ImGuiIO.
	io.IniFilename = nullptr; // No cache gui settings.
	io.LogFilename = nullptr; // No auto generated log texts.
	InitLayoutStyles();
}

ImGuiContextInstance::~ImGuiContextInstance()
{
	ImGui::DestroyContext(m_pImGuiContext);
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Context
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::SwitchCurrentContext() const
{
	ImGui::SetCurrentContext(m_pImGuiContext);
}

bool ImGuiContextInstance::IsActive() const
{
	return ImGui::GetCurrentContext() == m_pImGuiContext;
}

ImGuiIO& ImGuiContextInstance::GetIO() const
{
	return m_pImGuiContext->IO;
}

ImGuiPlatformIO& ImGuiContextInstance::GetPlatformIO() const
{
	return m_pImGuiContext->PlatformIO;
}

ImGuiStyle& ImGuiContextInstance::GetStyle() const
{
	return m_pImGuiContext->Style;
}

void ImGuiContextInstance::InitBackendUserData(void* pWindowManager, void* pRenderContext)
{
	ImGuiIO& io = GetIO();
	io.BackendPlatformUserData = pWindowManager;
	io.BackendRendererUserData = pRenderContext;
}

WindowManager* ImGuiContextInstance::GetWindowManager() const
{
	return static_cast<WindowManager*>(GetIO().BackendPlatformUserData);
}

RenderContext* ImGuiContextInstance::GetRenderContext() const
{
	return static_cast<RenderContext*>(GetIO().BackendRendererUserData);
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Display
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::SetDisplaySize(uint16_t width, uint16_t height)
{
	ImGuiIO& io = GetIO();
	io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
}

void ImGuiContextInstance::OnResize(uint16_t width, uint16_t height)
{
	SetDisplaySize(width, height);
}

bool ImGuiContextInstance::IsInsideDisplayRect(float x, float y) const
{
	ImGuiIO& io = GetIO();
	if (x < m_rectPosX ||
		x > m_rectPosX + io.DisplaySize.x ||
		y < m_rectPosY ||
		y > m_rectPosY + io.DisplaySize.y)
	{
		return false;
	}

	return true;
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

void ImGuiContextInstance::ClearUILayers()
{
	m_pImGuiStaticLayers.clear();
	m_pImGuiDockableLayers.clear();
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Dock
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::EnableDock()
{
	ImGuiIO& io = GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
}

bool ImGuiContextInstance::IsDockEnable() const
{
	return GetIO().ConfigFlags & ImGuiConfigFlags_DockingEnable;
}

void ImGuiContextInstance::BeginDockSpace()
{
	ImGuiIO& io = GetIO();

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
		ImGui::DockBuilderSetNodeSize(dockSpaceWindowID, io.DisplaySize * io.DisplayFramebufferScale);

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
		// TODO : Place these codes here is not suitable but convenient.
		ImGui::DockBuilderDockWindow("EntityList", dockSpaceUpLeftLeft);
		ImGui::DockBuilderDockWindow("GameView", dockSpaceLeftLeft);
		ImGui::DockBuilderDockWindow("SceneView", dockSpaceLeftRight);
		ImGui::DockBuilderDockWindow("SkeletonView", dockSpaceUpRight);
		ImGui::DockBuilderDockWindow("Profiler", dockSpaceUpRight);
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

///////////////////////////////////////////////////////////////////////////////////////////
/// Viewport
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::EnableViewport()
{
	ImGuiIO& io = GetIO();
	io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports | ImGuiBackendFlags_RendererHasViewports;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
}

bool ImGuiContextInstance::IsViewportEnable() const
{
	return GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable;
}

void ImGuiContextInstance::InitViewport()
{
	// Register window interfaces.
	static auto* s_pWindowManager = GetWindowManager();

	ImGuiPlatformIO& platformIO = GetPlatformIO();
	platformIO.Platform_CreateWindow = [](ImGuiViewport* pViewport)
	{
		auto pWindow = std::make_unique<engine::Window>("TempName", static_cast<int>(pViewport->Pos.x), static_cast<int>(pViewport->Pos.y),
			static_cast<int>(pViewport->Size.x), static_cast<int>(pViewport->Size.y));
		bool noDecoration = pViewport->Flags & ImGuiViewportFlags_NoDecoration;
		pWindow->SetBordedLess(noDecoration);
		pWindow->SetResizeable(!noDecoration);
		pViewport->PlatformHandle = pWindow.get();
		pViewport->PlatformHandleRaw = pWindow->GetHandle();
		s_pWindowManager->AddWindow(cd::MoveTemp(pWindow));
	};

	platformIO.Platform_DestroyWindow = [](ImGuiViewport* pViewport)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		s_pWindowManager->RemoveWindow(pWindow->GetID());
		pViewport->PlatformHandle = nullptr;
		pViewport->PlatformHandleRaw = nullptr;
	};

	platformIO.Platform_ShowWindow = [](ImGuiViewport* pViewport)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		pWindow->Show();
	};

	platformIO.Platform_SetWindowPos = [](ImGuiViewport* pViewport, ImVec2 v)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		pWindow->SetPosition(static_cast<int>(v.x), static_cast<int>(v.y));
	};

	platformIO.Platform_GetWindowPos = [](ImGuiViewport* pViewport) -> ImVec2
	{
		const auto* pWindow = static_cast<const engine::Window*>(pViewport->PlatformHandle);
		auto [x, y] = pWindow->GetPosition();
		return { static_cast<float>(x), static_cast<float>(y) };
	};

	platformIO.Platform_SetWindowSize = [](ImGuiViewport* pViewport, ImVec2 v)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		pWindow->SetSize(static_cast<int>(v.x), static_cast<int>(v.y));
	};

	platformIO.Platform_GetWindowSize = [](ImGuiViewport* pViewport) -> ImVec2
	{
		const auto* pWindow = static_cast<const engine::Window*>(pViewport->PlatformHandle);
		auto [w, h] = pWindow->GetSize();
		return { static_cast<float>(w), static_cast<float>(h) };
	};

	platformIO.Platform_SetWindowTitle = [](ImGuiViewport* pViewport, const char* pTitle)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		pWindow->SetTitle(pTitle);
	};

	platformIO.Platform_GetWindowFocus = [](ImGuiViewport* pViewport)
	{
		const auto* pWindow = static_cast<const engine::Window*>(pViewport->PlatformHandle);
		return pWindow->IsFocused();
	};
	
	platformIO.Platform_SetWindowFocus = [](ImGuiViewport* pViewport)
	{
		auto* pWindow = static_cast<engine::Window*>(pViewport->PlatformHandle);
		pWindow->SetFocused();
	};

	platformIO.Platform_GetWindowMinimized = [](ImGuiViewport* pViewport)
	{
		const auto* pWindow = static_cast<const engine::Window*>(pViewport->PlatformHandle);
		return pWindow->IsMinimized();
	};

	// Register rendering interfaces.
	static RenderContext* s_pRenderContext = GetRenderContext();
	platformIO.Renderer_CreateWindow = [](ImGuiViewport* pViewport)
	{
		pViewport->PlatformUserData = reinterpret_cast<void*>(s_pRenderContext->CreateView());
		engine::StringCrc newRenderTargetName = s_pRenderContext->GetRenderTargetCrc(pViewport->PlatformUserData);
		s_pRenderContext->CreateRenderTarget(newRenderTargetName, 1, 1, pViewport->PlatformHandleRaw);
	};

	platformIO.Renderer_DestroyWindow = [](ImGuiViewport* pViewport)
	{
		if (pViewport->PlatformUserData)
		{
			s_pRenderContext->DestoryRenderTarget(s_pRenderContext->GetRenderTargetCrc(pViewport->PlatformUserData));
			pViewport->PlatformUserData = nullptr;
		}
	};

	platformIO.Renderer_SetWindowSize = [](ImGuiViewport* pViewport, ImVec2 v)
	{
		auto* pRenderTarget = s_pRenderContext->GetRenderTarget(pViewport->PlatformUserData);
		pRenderTarget->Resize(static_cast<uint16_t>(v.x), static_cast<uint16_t>(v.y));
	};
}

void ImGuiContextInstance::UpdateViewport()
{
	ImGuiPlatformIO& platformIO = GetPlatformIO();
	int monitorCount = engine::Window::GetDisplayMonitorCount();
	platformIO.Monitors.resize(0);
	for (int monitorIndex = 0; monitorIndex < monitorCount; ++monitorIndex)
	{
		auto mainRect = engine::Window::GetDisplayMonitorMainRect(monitorIndex);
		auto workRect = engine::Window::GetDisplayMonitorWorkRect(monitorIndex);

		ImGuiPlatformMonitor monitor;
		monitor.MainPos = ImVec2((float)mainRect.x, (float)mainRect.y);
		monitor.MainSize = ImVec2((float)mainRect.w, (float)mainRect.h);
		monitor.WorkPos = ImVec2((float)workRect.x, (float)workRect.y);
		monitor.WorkSize = ImVec2((float)workRect.w, (float)workRect.h);

		// Check if the display's position is at (0,0), which is typical for the primary display.
		bool isPrimaryDisplay = mainRect.x == 0 && mainRect.y == 0;
		if (isPrimaryDisplay)
		{
			platformIO.Monitors.push_front(monitor);
		}
		else
		{
			platformIO.Monitors.push_back(monitor);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Styles
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language)
{
	ImGuiIO& io = GetIO();
	io.FontGlobalScale = 1.0f;

	ImFontConfig config;
	config.OversampleH = 2;
	config.OversampleV = 1;
	config.GlyphExtraSpacing.x = 1.0f;

	bool ttfFileLoaded = false;
	for (const std::string& ttfFileName : ttfFileNames)
	{
		//std::string editorFontResourcePath = std::format("{}/Font/{}", CDEDITOR_RESOURCES_ROOT_PATH, ttfFileName);
		std::string editorFontResourcePath = CDEDITOR_RESOURCES_ROOT_PATH;
		editorFontResourcePath += "/Font/";
		editorFontResourcePath += ttfFileName;

		const ImWchar* pGlyphRanges = nullptr;
		switch (language)
		{
		// Glyph ranges for Chinese Simplied have issues on showing some characters.
		// So let's use the bigger Glyph ranges including tradional characters.
		case engine::Language::ChineseSimplied:
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

	ImFontAtlas* pFontAtlas = io.Fonts;
#ifdef IMGUI_ENABLE_FREETYPE
	pFontAtlas->FontBuilderIO = ImGuiFreeType::GetBuilderForFreeType();
#endif
	pFontAtlas->FontBuilderFlags = 0;
	pFontAtlas->Build();
}

void ImGuiContextInstance::InitLayoutStyles()
{
	ImGuiStyle& style = GetStyle();
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

	if (IsViewportEnable())
	{
		style.WindowRounding = roundingAmount;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
}

void ImGuiContextInstance::SetImGuiThemeColor(ThemeColor theme)
{
	m_themeColor = theme;

	ImGuiStyle& style = GetStyle();
	ImVec4* colours = style.Colors;
	if (ThemeColor::Black == theme)
	{
		ImGui::StyleColorsDark(&style);
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
		ImGui::StyleColorsClassic(&style);
	}
	else if (ThemeColor::Dark == theme)
	{
		ImGui::StyleColorsDark(&style);
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
		ImGui::StyleColorsDark(&style);
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
		ImGui::StyleColorsLight(&style);
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

void ImGuiContextInstance::SetImGuiLanguage(Language language)
{
	m_language = language;
	Localization::SetLanguage(language);
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Input
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::AddInputEvent()
{
	// Don't push unnecessary events to ImGui. Or it will overflow its event queue to cause a time delay UI feedback.
	// Why we don't use callback directly from native platform window? Because we have multiple imgui contexts to receive input events.
	// And only the active imgui context can receive window messages. It sounds no problem but imgui context can switch during one frame multiple times.
	// It is not safe to use event callback.
	AddMouseInputEvent();
	AddKeyboardInputEvent();
}

void ImGuiContextInstance::AddMouseInputEvent()
{
	ImGuiIO& io = GetIO();

	// TODO : is this focus event for this context?
	if (bool isFocused = Input::Get().IsFocused(); isFocused != m_lastFocused)
	{
		io.AddFocusEvent(isFocused);
		m_lastFocused = isFocused;
	}

	float mousePosX;
	float mousePosY;
	if (IsViewportEnable())
	{
		// Multiple Viewports require screen space mouse coordinates.
		auto [screenX, screenY] = engine::Input::GetGloalMousePosition();
		mousePosX = static_cast<float>(screenX);
		mousePosY = static_cast<float>(screenY);
	}
	else
	{
		mousePosX = static_cast<float>(Input::Get().GetMousePositionX());
		mousePosY = static_cast<float>(Input::Get().GetMousePositionY());
	}

	// Filter mouse events outside of rect.
	bool isInsideDisplayRect = IsInsideDisplayRect(mousePosX, mousePosY);
	if (isInsideDisplayRect != m_lastInsideDisplayRect)
	{
		if (isInsideDisplayRect)
		{
			OnMouseEnterDisplayRect.Invoke();
		}
		else
		{
			OnMouseLeaveDisplayRect.Invoke();
		}

		m_lastInsideDisplayRect = isInsideDisplayRect;
	}

	if (!isInsideDisplayRect)
	{
		return;
	}

	if (mousePosX != m_lastMousePositionX || mousePosY != m_lastMousePositionY)
	{
		io.AddMousePosEvent(mousePosX, mousePosY);
		m_lastMousePositionX = mousePosX;
		m_lastMousePositionY = mousePosY;
	}

	if (bool mouseLBPressed = Input::Get().IsMouseLBPressed(); m_lastMouseLBPressed != mouseLBPressed)
	{
		io.AddMouseButtonEvent(ImGuiMouseButton_Left, mouseLBPressed);
		m_lastMouseLBPressed = mouseLBPressed;
	}

	if (bool mouseRBPressed = Input::Get().IsMouseRBPressed(); m_lastMouseRBPressed != mouseRBPressed)
	{
		io.AddMouseButtonEvent(ImGuiMouseButton_Right, mouseRBPressed);
		m_lastMouseRBPressed = mouseRBPressed;
	}

	if (bool mouseMBPressed = Input::Get().IsMouseMBPressed(); m_lastMouseMBPressed != mouseMBPressed)
	{
		io.AddMouseButtonEvent(ImGuiMouseButton_Middle, mouseMBPressed);
		m_lastMouseMBPressed = mouseMBPressed;
	}

	if (float mouseScrollOffsetY = Input::Get().GetMouseScrollOffsetY(); mouseScrollOffsetY != m_lastMouseScrollOffstY)
	{
		io.AddMouseWheelEvent(0.0f, mouseScrollOffsetY);
		m_lastMouseScrollOffstY = mouseScrollOffsetY;
	}
}

void ImGuiContextInstance::AddKeyboardInputEvent()
{
	ImGuiIO& io = GetIO();

	const std::vector<Input::KeyEvent> keyEvents = Input::Get().GetKeyEventList();
	for (uint32_t i = 0; i < keyEvents.size(); ++i)
	{
		const Input::KeyEvent keyEvent = keyEvents[i];
		if (keyEvent.mod != KeyMod::KMOD_NONE)
		{
			// Add the modifier key event
			if (kImguiKeyModToImGuiModLookup.find(keyEvent.mod) != kImguiKeyModToImGuiModLookup.cend())
			{
				io.AddKeyEvent(kImguiKeyModToImGuiModLookup[keyEvent.mod], keyEvent.isPressed);
			}
			// Also add the key itself as key event
			if (kImguiKeyModToImGuiKeyLookup.find(keyEvent.mod) != kImguiKeyModToImGuiKeyLookup.cend())
			{
				io.AddKeyEvent(kImguiKeyModToImGuiKeyLookup[keyEvent.mod], keyEvent.isPressed);
			}
		}
		if (kImguiKeyLookup.find(keyEvent.code) != kImguiKeyLookup.cend()) {
			io.AddKeyEvent(kImguiKeyLookup[keyEvent.code], keyEvent.isPressed);
		}
	}

	const char* inputChars = Input::Get().GetInputCharacters();
	const size_t inputCharSize = strlen(inputChars);
	for (size_t i = 0; i < inputCharSize; ++i)
	{
		io.AddInputCharacter(inputChars[i]);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
/// Loop
///////////////////////////////////////////////////////////////////////////////////////////
void ImGuiContextInstance::BeginFrame()
{
	SwitchCurrentContext();
}

void ImGuiContextInstance::EndFrame()
{
	assert(IsActive());

	if (IsViewportEnable())
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void ImGuiContextInstance::Update(float deltaTime)
{
	assert(IsActive());

	auto& io = GetIO();

	// It is necessary to pass correct deltaTime to ImGui underlaying framework because it will use the value to check
	// something such as if mouse button double click happens(Two click event happens in one frame, < deltaTime).
	io.DeltaTime = deltaTime;

	if (IsViewportEnable())
	{
		UpdateViewport();
	}

	AddInputEvent();

	ImGui::NewFrame();

	for (const auto& pImGuiLayer : m_pImGuiStaticLayers)
	{
		pImGuiLayer->Update();
	}

	const bool dockingEnabled = io.ConfigFlags & ImGuiConfigFlags_DockingEnable;
	if (dockingEnabled)
	{
		BeginDockSpace();
	}

	for (const auto& pImGuiLayer : m_pImGuiDockableLayers)
	{
		if (pImGuiLayer->IsEnable())
		{
			pImGuiLayer->Update();
		}
	}

	if (dockingEnabled)
	{
		EndDockSpace();
	}
}

}