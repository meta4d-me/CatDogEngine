#pragma once

namespace engine
{

class IGame
{
public:
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void Shutdown() = 0;
};

}