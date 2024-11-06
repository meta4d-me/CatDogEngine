#pragma once

#ifdef SPDLOG_ENABLE

#include "Math/Quaternion.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bundled/format.h>

namespace engine
{

class Log
{
public:
	static void Init();
	static std::shared_ptr<spdlog::logger> GetEngineLogger() { return s_pEngineLogger; }
	static std::shared_ptr<spdlog::logger> GetApplicationLogger() { return s_pApplicationLogger; }

	static const std::ostringstream& GetSpdOutput() { return m_oss; }
	static void ClearBuffer();

private:
	static std::shared_ptr<spdlog::logger> s_pEngineLogger;
	static std::shared_ptr<spdlog::logger> s_pApplicationLogger;

	// Note that m_oss will be cleared after OutputLog::AddSpdLog be called.
	static std::ostringstream m_oss;
};

}

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

// Runtime assert.
#define CD_ENGINE_ASSERT(x, ...) { if(!(x)) { CD_FATAL(...); __debugbreak(); } }
#define CD_ASSERT(x, ...) { if(!(x)) { CD_FATAL(...); __debugbreak(); } }

template<>
struct fmt::formatter<cd::Vec2f> : fmt::formatter<std::string>
{
	auto format(const cd::Vec2f &vec, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "vec2:({}, {})", vec.x(), vec.y());
	}
};

template<>
struct fmt::formatter<cd::Vec3f> : fmt::formatter<std::string>
{
	auto format(const cd::Vec3f &vec, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "vec3:({}, {}, {})", vec.x(), vec.y(), vec.z());
	}
};

template<>
struct fmt::formatter<cd::Vec4f> : fmt::formatter<std::string>
{
	auto format(const cd::Vec4f &vec, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "vec4:({}, {}, {}, {})", vec.x(), vec.y(), vec.z(), vec.w());
	}
};

template<>
struct fmt::formatter<cd::Quaternion> : fmt::formatter<std::string>
{
	auto format(const cd::Quaternion &qua, format_context &ctx) const -> decltype(ctx.out())
	{
		return fmt::format_to(ctx.out(), "Vector = ({}, {}, {}), Scalar = {}", qua.x(), qua.y(), qua.z(), qua.w());
	}
};

#else

namespace engine
{
class Log
{
public:
	static void Init() {}
};
}

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

#define CD_ENGINE_ASSERT(x, ...)
#define CD_ASSERT(x, ...)

#endif