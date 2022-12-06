#pragma once

namespace engine
{

class EditorApp;

class ImGuiBaseLayer
{
public:
	ImGuiBaseLayer(const char* pName) : m_pName(pName) { }
	ImGuiBaseLayer(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer& operator=(const ImGuiBaseLayer&) = delete;
	ImGuiBaseLayer(ImGuiBaseLayer&&) = default;
	ImGuiBaseLayer& operator=(ImGuiBaseLayer&&) = default;
	virtual ~ImGuiBaseLayer() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	const char* GetName() const { return m_pName; }

	void SetEnable(bool enable) { m_isEnable = enable; }
	bool IsEnable() const { return m_isEnable; }

protected:
	const char* m_pName = nullptr;
	bool m_isEnable = true;
};

}