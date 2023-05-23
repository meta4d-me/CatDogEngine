#include "EditorApp.h"
#include "Application/Engine.h"

int main()
{
	using namespace engine;
	Engine* pEngine = Engine::Create(std::make_unique<editor::EditorApp>());

	EngineInitArgs initArgs;
	pEngine->Init({ .pTitle = "CatDogEditor", .pIconFilePath = "editor_icon.png",
		.width = 1920, .height = 1080, .useFullScreen = false,
		.language = Language::ChineseSimplied, .backend = GraphicsBackend::Direct3D11 });

	pEngine->Run();

	Engine::Destroy(pEngine);

	return 0;
}