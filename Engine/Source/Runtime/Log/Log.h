#pragma once

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#include "Math/Vector.hpp"

#include <format>

namespace engine
{

class Log
{
public:
	static void Init();

	static std::shared_ptr<spdlog::logger> &GetEngineLogger() { return s_engineLogger; }
	static std::shared_ptr<spdlog::logger> &GetApplicationLogger() { return s_applicationLogger; }

private:
	static std::shared_ptr<spdlog::logger> s_engineLogger;
	static std::shared_ptr<spdlog::logger> s_applicationLogger;
};

} // namespace engine

inline std::ostream &operator<<(std::ostream &os, const cd::Vec2f &vec)
{
	return os << std::format("({0}, {1})", vec.x(), vec.y());
}

inline std::ostream &operator<<(std::ostream &os, const cd::Vec3f &vec)
{
	return os << std::format("({0}, {1}, {2})", vec.x(), vec.y(), vec.z());
}

inline std::ostream &operator<<(std::ostream &os, const cd::Vec4f &vec)
{
	return os << std::format("({0}, {1}, {2}, {3})", vec.x(), vec.y(), vec.z(), vec.w());
}

#ifndef NDEBUG

// Engine log macros.
#define CD_ENGINE_TRACE(...) ::engine::Log::GetEngineLogger()->trace(__VA_ARGS__)
#define CD_ENGINE_INFO(...)  ::engine::Log::GetEngineLogger()->info(__VA_ARGS__)
#define CD_ENGINE_WARN(...)  ::engine::Log::GetEngineLogger()->warn(__VA_ARGS__)
#define CD_ENGINE_ERROR(...) ::engine::Log::GetEngineLogger()->error(__VA_ARGS__)
#define CD_ENGINE_FATAL(...) ::engine::Log::GetEngineLogger()->critical(__VA_ARGS__)

// Application log macros.
#define CD_TRACE(...) ::engine::Log::GetApplicationLogger()->trace(__VA_ARGS__)
#define CD_INFO(...)  ::engine::Log::GetApplicationLogger()->info(__VA_ARGS__)
#define CD_WARN(...)  ::engine::Log::GetApplicationLogger()->warn(__VA_ARGS__)
#define CD_ERROR(...) ::engine::Log::GetApplicationLogger()->error(__VA_ARGS__)
#define CD_FATAL(...) ::engine::Log::GetApplicationLogger()->critical(__VA_ARGS__)

#else

// Maby we don't need log in release mode.
#define CD_ENGINE_TRACE(...)
#define CD_ENGINE_INFO(...) 
#define CD_ENGINE_WARN(...) 
#define CD_ENGINE_ERROR(...)
#define CD_ENGINE_FATAL(...)
#define CD_TRACE(...)
#define CD_INFO(...) 
#define CD_WARN(...) 
#define CD_ERROR(...)
#define CD_FATAL(...)

#endif
