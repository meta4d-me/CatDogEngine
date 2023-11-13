#pragma once

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
	void InitBackendUserData(void* pWindow, void* pRenderContext);

	// Display
	void SetRectPosition(float x, float y) { m_rectPosX = x; m_rectPosY = y; }
	void SetDisplaySize(uint16_t width, uint16_t height);
	void OnResize(uint16_t width, uint16_t height);
	bool IsInsideDisplayRect(float x, float y) const;
	void AddStaticLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	std::vector<std::unique_ptr<ImGuiBaseLayer>>& GetDockableLayers() { return m_pImGuiDockableLayers; }
	void AddDynamicLayer(std::unique_ptr<ImGuiBaseLayer> pLayer);
	ImGuiBaseLayer* GetLayerByName(StringCrc nameCrc) const;
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
	void InitViewport(WindowManager* pWindowManager, RenderContext* pRenderContext);
	void UpdateViewport();

	// Loop.
	void BeginFrame();
	void Update(float deltaTime);
	void EndFrame();

	// Access to world data.
	void SetSceneWorld(SceneWorld* pSceneWorld) { m_pSceneWorld = pSceneWorld; }
	SceneWorld* GetSceneWorld() const { return m_pSceneWorld; }

private:
	void InitLayoutStyles();

	void BeginDockSpace();
	void EndDockSpace();

	// Input
	void AddInputEvent();
	void AddMouseInputEvent();
	void AddKeyboardInputEvent();

private:
	SceneWorld* m_pSceneWorld = nullptr;
	ImGuiContext* m_pImGuiContext = nullptr;
	Language m_language;
	ThemeColor m_themeColor;

	float m_rectPosX = 0.0f;
	float m_rectPosY = 0.0f;

	bool m_lastMouseLBPressed = false;
	bool m_lastMouseRBPressed = false;
	bool m_lastMouseMBPressed = false;
	float m_lastMouseScrollOffstY = 0.0f;
	float m_lastMousePositionX = 0.0f;
	float m_lastMousePositionY = 0.0f;

	std::map<StringCrc, ImGuiBaseLayer*> m_mapNameCrcToLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiStaticLayers;
	std::vector<std::unique_ptr<ImGuiBaseLayer>> m_pImGuiDockableLayers;
};

}