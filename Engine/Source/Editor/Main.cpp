#include "EditorApp.h"
#include "Application/Engine.h"

int main()
{
	using namespace engine;
	Engine* pEngine = Engine::Create(std::make_unique<editor::EditorApp>());

	EngineInitArgs initArgs;
	initArgs.pTitle = "CatDogEditor";
	initArgs.pIconFilePath = "editor_icon.png";
	initArgs.language = Language::ChineseSimplied;
	initArgs.width = 1200;
	initArgs.height = 600;
	pEngine->Init(std::move(initArgs));

	pEngine->Run();

	Engine::Destroy(pEngine);

	return 0;
}