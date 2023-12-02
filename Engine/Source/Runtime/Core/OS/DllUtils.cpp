#include "DllUtils.h"

#include <SDL.h>

namespace engine
{

void* DllUtils::LoadDll(const char* pFilePath)
{
	return SDL_LoadObject(pFilePath);
}

void* DllUtils::LoadDllFunction(void* pHandle, const char* pFunctionName)
{
	return SDL_LoadFunction(pHandle, pFunctionName);
}

void DllUtils::UnloadDll(void* pHandle)
{
	return SDL_UnloadObject(pHandle);
}

}