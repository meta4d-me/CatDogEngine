#pragma once

#include <inttypes.h>

namespace engine
{

class Engine;

struct EngineInitArgs
{
public:
	const char* pTitle = "Demo";
	uint16_t width = 800;
	uint16_t height = 600;
};

class IApplication
{
public:
	explicit IApplication() = default;
	IApplication(const IApplication&) = delete;
	IApplication& operator=(const IApplication&) = delete;
	IApplication(IApplication&&) = default;
	IApplication& operator=(IApplication&&) = default;
	virtual ~IApplication() {}

	virtual void Init(EngineInitArgs initArgs) = 0;
	virtual bool Update(float deltaTime) = 0;
	virtual void Shutdown() = 0;

	void SetEngine(Engine* pEngine) { m_pEngine = pEngine; }
	Engine* GetEngine() { return m_pEngine; };

protected:
	Engine* m_pEngine;
};

}