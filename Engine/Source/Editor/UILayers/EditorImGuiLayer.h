#pragma once

namespace editor
{

class EditorApp;

class EditorImGuiLayer
{
public:
	EditorImGuiLayer(const char* pName, EditorApp* pEditorApp) : m_pName(pName), m_pEditorApp(pEditorApp) { }
	EditorImGuiLayer(const EditorImGuiLayer&) = delete;
	EditorImGuiLayer& operator=(const EditorImGuiLayer&) = delete;
	EditorImGuiLayer(EditorImGuiLayer&&) = default;
	EditorImGuiLayer& operator=(EditorImGuiLayer&&) = default;
	virtual ~EditorImGuiLayer() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;

	const char* GetName() const { return m_pName; }

	void SetEnable(bool enable) { m_isEnable = enable; }
	bool IsEnable() const { return m_isEnable; }

protected:
	const char* m_pName = nullptr;
	bool m_isEnable = true;
	EditorApp* m_pEditorApp = nullptr;
};

}