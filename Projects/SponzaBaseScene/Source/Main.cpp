#include "Engine.h"

static engine::Engine* s_pEngine = nullptr;

int main()
{
	s_pEngine = new engine::Engine();
	s_pEngine->InitPlatformWindow("SponzarBaseScene", 800, 600);
	s_pEngine->Init();
	s_pEngine->MainLoop();
	s_pEngine->Shutdown();

	return 0;
}