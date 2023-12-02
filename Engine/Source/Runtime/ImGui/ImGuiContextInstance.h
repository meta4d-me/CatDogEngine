#pragma once

#include "Core/Delegates/Delegate.hpp"
#include "Core/Delegates/MulticastDelegate.hpp"
#include "Core/StringCrc.h"
#include "Localization.h"
#include "ThemeColor.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct ImGuiContext;
struct ImGuiIO;
struct ImGuiPlatformIO;
struct ImGuiStyle;

namespace engine
{

class ImGuiBaseLayer;
class ImGuiContextManager;
class RenderContext;
class SceneWorld;
class WindowManager;

class ImGuiContextInstance
{
public:
	ImGuiContextInstance();
	ImGuiContextInstance(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance& operator=(const ImGuiContextInstance&) = delete;
	ImGuiContextInstance(ImGuiContextInstance&&) = default;
	ImGuiContextInstance& operator=(ImGuiContextInstance&&) = default;
	virtual ~ImGuiContextInstance();

	// Context
	void SwitchCurrentContext() const;
	bool IsActive() const;
	ImGuiContext* GetContext() const { return m_pImGuiContext; }
	ImGuiIO& GetIO() const;
	ImGuiPlatformIO& GetPlatformIO() const;
	ImGuiStyle& GetStyle() const;
	void SetContextManager(ImGuiContextManager* pManager) { m_pImGuiContextManager = pManager; }
	ImGuiContextManager* GetContextManager() const { return m_pImGuiContextManager; }
	void InitBackendUserData(void* pWindowManager, void* pRenderContext);
	WindowManager* GetWindowManager() const;
	RenderContext* GetRenderContext() const;

	// Display
	void SetRectPosition(float x, float y) { m_rectPosX = x; m_rectPosY = y; }
	std::pair<float, float> GetRectPosition() { return std::make_pair(m_rectPosX, m_rectPosY); }
	void SetDisplaySize(uint16_t width, uint16_t height);
	void OnResize(uint16_t width, uint16_t height);
	bool IsInsideDisplayRect(float x, float y) const;
	const std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetStaticLayers() const { return m_pImGuiStaticLayers; }
	void AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	const std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetDynamicLayers() const { return m_pImGuiDockableLayers; }
	void AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	void ClearUILayers();

	// Styles
	void LoadFontFiles(const std::vector<std::string>& ttfFileNames, engine::Language language);
	ThemeColor GetImGuiThemeColor() const { return m_themeColor; }
	void SetImGuiThemeColor(ThemeColor theme);
	Language GetImGuiLanguage() const { return m_language; }
	void SetImGuiLanguage(Language language);

	// Docking features.
	void EnableDock();
	bool IsDockEnable() const;

	// Viewport feature.
	void EnableViewport();
	bool IsViewportEnable() const;
	void InitViewport();
	void UpdateMonitors();

	// Loop.
	void BeginFrame();
	void Update(float deltaTime);
	void EndFrame();

	// Access to world data.
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pSceneWorld = pSceneWorld; }
	SceneWorld* GetSceneWorld() const { return m_pSceneWorld; }

	// Delegates
	MulticastDelegate<bool(float)> OnMouseWheel;
	MulticastDelegate<bool(float, float)> OnMouseMove;
	MulticastDelegate<bool(float, float)> OnMouseDown;
	MulticastDelegate<bool(float, float)> OnMouseUp;
	MulticastDelegate<bool(void)> OnKeyDown;
	Delegate<void(void)> OnMouseEnterDisplayRect;
	Delegate<void(void)> OnMouseLeaveDisplayRect;

private:
	void InitLayoutStyles();

	void BeginDockSpace();
	void EndDockSpace();

	// Events
	void AddInputEvents();
	void AddMouseInputEvents();
	void AddKeyboardInputEvents();
	void PopulateEvents();

private:
	SceneWorld* m_pSceneWorld = nullptr;
	ImGuiContext* m_pImGuiContext = nullptr;
	ImGuiContextManager* m_pImGuiContextManager = nullptr;
	Language m_language;
	ThemeColor m_themeColor;

	float m_rectPosX = 0.0f;
	float m_rectPosY = 0.0f;
	
	bool m_isAnyKeyDown = false;

	bool m_lastInsideDisplayRect = false;
	bool m_lastFocused = false;
	bool m_lastMouseLBPressed = false;
	bool m_lastMouseRBPressed = false;
	bool m_lastMouseMBPressed = false;
	float m_lastMouseScrollOffstY = 0.0f;
	float m_lastMousePositionX = 0.0f;
	float m_lastMousePositionY = 0.0f;

	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiDockableLayers;
};

}