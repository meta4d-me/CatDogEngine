#include "MainMenu.h"

#include "EditorApp.h"
#include "ImGui/ImGuiContextInstance.h"
#include "ImGui/ThemeColor.h"
#include "Log/Log.h"
#include "Resources/ResourceBuilder.h"
#include "Window/Window.h"
#include "Window/Input.h"
#include "Window/KeyCode.h"

#include <imgui/imgui.h>
#include<string>
#include <filesystem>
#include "ImGui/imfilebrowser.h"
#include "ImGui/EditorImGuiViewport.h"





namespace editor
{

	

MainMenu::~MainMenu()
{
	if (m_pCreatProject)
	{
		delete m_pCreatProject;
		m_pCreatProject = nullptr;
	}
}

void MainMenu::FileMenu()
{
	
	
	
	
	if (ImGui::BeginMenu("File"))
	{ 
		
		if (ImGui::MenuItem("New", "Ctrl N"))
		{
			m_pCreatProject->SetTitle("Creat - Project");
			m_pCreatProject->Open();
		}
		if (ImGui::MenuItem("New", "Ctrl O"))
		{
		}
		if (ImGui::MenuItem("Open Recent"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Save", "Ctrl S"))
		{
		}
		if (ImGui::MenuItem("Save As", "Shift Ctrl S"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Quit", "Ctrl Q"))
		{
			if (auto* pMainWindow = reinterpret_cast<engine::Window*>(ImGui::GetIO().BackendPlatformUserData))
			{
				pMainWindow->Closed();
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::EditMenu()
{
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::BeginMenu("Edit"))
	{
		if (ImGui::MenuItem("Undo", "Ctrl Z"))
		{
		}
		if (ImGui::MenuItem("Redo", "Shift Ctrl Z"))
		{
		}

		ImGui::Separator();

		if (ImGui::MenuItem("Preference"))
		{
		}

		if (ImGui::BeginMenu("Style"))
		{
			// It is not convenient in C++ to loop enum except define an extra array to wrap them.
			// C++ 20/23 ranges may look better but still needs std::iota inside its implementation.
			for (engine::ThemeColor theme = engine::ThemeColor::Black; theme < engine::ThemeColor::Count;
				theme = static_cast<engine::ThemeColor>(static_cast<int>(theme) + 1))
			{
				engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
				if (ImGui::MenuItem(GetThemeColorName(theme), "", pImGuiContextInstance->GetImGuiThemeColor() == theme))
				{
					pImGuiContextInstance->SetImGuiThemeColor(theme);
				}
			}

			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Language"))
		{
			for (engine::Language language = engine::Language::ChineseSimplied; language < engine::Language::Count;
				language = static_cast<engine::Language>(static_cast<int>(language) + 1))
			{
				engine::ImGuiContextInstance* pImGuiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
				if (ImGui::MenuItem(engine::GetLanguageName(language), "", pImGuiContextInstance->GetImGuiLanguage() == language))
				{
					pImGuiContextInstance->SetImGuiLanguage(language);
				}
				
			}
			ImGui::EndMenu();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::WindowMenu()
{
	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::BeginMenu("Window"))
	{
		engine::ImGuiContextInstance* pCurrentImguiContextInstance = reinterpret_cast<engine::ImGuiContextInstance*>(io.UserData);
		for (const auto& pDockableLayer : pCurrentImguiContextInstance->GetDockableLayers())
		{
			if (ImGui::MenuItem(pDockableLayer->GetName(), "", pDockableLayer->IsEnable()))
			{
				pDockableLayer->SetEnable(!pDockableLayer->IsEnable());
			}
		}

		ImGui::EndMenu();
	}
}

void MainMenu::BuildMenu()
{
	if (ImGui::BeginMenu("Build"))
	{
		if (ImGui::MenuItem("Rebuild Shaders"))
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(CDENGINE_BUILTIN_SHADER_PATH))
			{
				const auto& filePath = entry.path();
				if (".sc" != filePath.extension())
				{
					continue;
				}

				ShaderType shaderType;
				std::string fileName = filePath.stem().generic_string();
				if (fileName.starts_with("vs_") || fileName.starts_with("VS_"))
				{
					shaderType = ShaderType::Vertex;
				}
				else if (fileName.starts_with("fs_") || fileName.starts_with("FS_"))
				{
					shaderType = ShaderType::Fragment;
				}
				else if (fileName.starts_with("cs_") || fileName.starts_with("CS_"))
				{
					shaderType = ShaderType::Compute;
				}
				else
				{
					CD_ERROR("Shader source file's type is unknown by its file name : {0}.", fileName.c_str());
					continue;
				}
				
				// TODO : We don't know their uber options here.
				// So this feature only generates default shader now.
				std::string outputShaderPath = CDENGINE_RESOURCES_ROOT_PATH;
				outputShaderPath += "Shaders/" + filePath.stem().generic_string();
				outputShaderPath += ".bin";
				ResourceBuilder::Get().AddShaderBuildTask(shaderType,
					filePath.generic_string().c_str(), outputShaderPath.c_str());
			}

			ResourceBuilder::Get().Update();
		}

		ImGui::EndMenu();
	}
}

void MainMenu::AboutMenu()
{
	if (ImGui::BeginMenu("About"))
	{
		if (ImGui::MenuItem("Documents"))
		{
		}

		ImGui::EndMenu();
	}
}

void MainMenu::Init()
{
	m_pCreatProject = new ImGui::FileBrowser();
}

void MainMenu::Update()
{
	if (ImGui::BeginMainMenuBar())
	{
		FileMenu();
		EditMenu();
		WindowMenu();
		BuildMenu();
		AboutMenu();
		ImGui::EndMainMenuBar();
	}
	m_pCreatProject->Display();

	if (engine::Input::Get().ContainsModifier(engine::KeyMod::KMOD_CTRL) 
		&& engine::Input::Get().IsKeyPressed(engine::KeyCode::q))
	{
		if (auto* pMainWindow = reinterpret_cast<engine::Window*>(ImGui::GetIO().BackendPlatformUserData))
		{ 
			pMainWindow->Closed();
		}
	}
}


}