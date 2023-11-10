#pragma once

namespace engine
{

// After loading a dll, we need to call an Init function to create inherited IModule object.
// So there is a rule about this Init function's name and ABI declaration.
static constexpr const char* ModuleInitFunctioName = "InitializeModule";
using ModuleInitFunctionPtr = void* (*)(void);

class IModule
{
public:
	virtual ~IModule() {}

	void Shutdown();
};

}