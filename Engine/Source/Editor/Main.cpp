#include "EditorApp.h"
#include "Application/Engine.h"

int main()
{
	using namespace engine;
	Engine* pEngine = Engine::Create(std::make_unique<editor::EditorApp>());

	pEngine->Init({ .pTitle = "CatDogEditor", .pIconFilePath = "editor_icon.png",
		.width = 1280, .height = 720, .useFullScreen = false,
		.language = Language::ChineseSimplied,
		.backend = GraphicsBackend::OpenGL, .compileAllShaders = false });

	pEngine->Run();

	Engine::Destroy(pEngine);

	return 0;
}