#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace engine
{

class Log
{
public:
	static void Init();

	static std::shared_ptr<spdlog::logger> &GetEngineLogger() { return s_engineLogger; }
	static std::shared_ptr<spdlog::logger> &GetEditorLogger() { return s_editorLogger; }

private:
	static std::shared_ptr<spdlog::logger> s_engineLogger;
	static std::shared_ptr<spdlog::logger> s_editorLogger;
};

} // namespace engine

#ifndef NDEBUG

// Engine log macros.
#define CD_ENGINE_TRACE(...) ::engine::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define CD_ENGINE_INFO(...)  ::engine::Log::GetEngineLogger()->info(__VA_ARGS__)
#define CD_ENGINE_WARN(...)  ::engine::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define CD_ENGINE_ERROR(...) ::engine::Log::GetEngineLogger()->error(__VA_ARGS__)
#define CD_ENGINE_FATAL(...) ::engine::Log::GetEngineLogger()->critical(__VA_ARGS__)

// Editor log macros.
#define CD_EDITOR_TRACE(...) ::engine::Log::GetEditorLogger()->trace(__VA_ARGS__)
#define CD_EDITOR_INFO(...)  ::engine::Log::GetEditorLogger()->info(__VA_ARGS__)
#define CD_EDITOR_WARN(...)  ::engine::Log::GetEditorLogger()->warn(__VA_ARGS__)
#define CD_EDITOR_ERROR(...) ::engine::Log::GetEditorLogger()->error(__VA_ARGS__)
#define CD_EDITOR_FATAL(...) ::engine::Log::GetEditorLogger()->critical(__VA_ARGS__)

#else

// Maby we don't need log in release mode.
#define CD_ENGINE_TRACE(...)
#define CD_ENGINE_INFO(...) 
#define CD_ENGINE_WARN(...) 
#define CD_ENGINE_ERROR(...)
#define CD_ENGINE_FATAL(...)
#define CD_EDITOR_TRACE(...)
#define CD_EDITOR_INFO(...) 
#define CD_EDITOR_WARN(...) 
#define CD_EDITOR_ERROR(...)
#define CD_EDITOR_FATAL(...)

#endif
