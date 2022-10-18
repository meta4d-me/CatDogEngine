#include "Engine.h"

#include <memory>

static std::unique_ptr<engine::Engine> s_pEngine;

int main()
{
	s_pEngine = std::make_unique<engine::Engine>();
	s_pEngine->InitPlatformWindow("SponzarBaseScene", 1200, 900);
	s_pEngine->Init();
	s_pEngine->MainLoop();
	s_pEngine->Shutdown();

	return 0;
}