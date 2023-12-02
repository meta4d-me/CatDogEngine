#pragma once

#include "Core/StringCrc.h"

#include <optional>
#include <utility>

struct ImGuiWindow;

namespace engine
{

class ImGuiContextInstance;
class ImGuiContextManager;
class RenderContext;
class SceneWorld;

class ImGuiBaseLayer
{
public:
	ImGuiBaseLayer() = delete;
	explicit ImGuiBaseLayer(const char* pName);
	ImGuiBaseLayer(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer& operator=(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer(ImGuiBaseLayer&&) = default;
	ImGuiBaseLayer& operator=(ImGuiBaseLayer&&) = default;
	virtual ~ImGuiBaseLayer() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	const char* GetName() const { return m_pName; }
	uint32_t GetID() const { return m_id; }

	ImGuiWindow* GetRootWindow() const;
	virtual std::pair<float, float> GetRectPosition() const;
	virtual std::pair<float, float> GetRectSize() const;
	virtual std::pair<float, float> GetWorkRectPosition() const;
	virtual std::pair<float, float> GetWorkRectSize() const;

	void SetEnable(bool enable) { m_isEnable = enable; }
	bool IsEnable() const { return m_isEnable; }

	ImGuiContextInstance* GetImGuiContextInstance() const;
	ImGuiContextManager* GetImGuiContextManager() const;
	ImGuiBaseLayer* GetImGuiLayer(StringCrc nameCrc) const;
	RenderContext* GetRenderContext() const;
	SceneWorld* GetSceneWorld() const;

protected:
	const char* m_pName = nullptr;
	uint32_t m_id = UINT32_MAX;
	bool m_isEnable = true;
};

}