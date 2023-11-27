#pragma once

namespace engine
{

class DllUtils
{
public:
	DllUtils() = delete;

	static void* LoadDll(const char* pFilePath);
	static void* LoadDllFunction(void* pHandle, const char* pFunctionName);
	static void UnloadDll(void* pHandle);
};

}