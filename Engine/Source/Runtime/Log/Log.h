#pragma once

#ifdef SPDLOG_ENABLE

#include "Math/Quaternion.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>

#include <sstream>

namespace engine
{

class Log
{
public:
	static void Init();
	static std::shared_ptr<spdlog::logger>& GetEngineLogger();
	static std::shared_ptr<spdlog::logger>& GetApplicationLogger();

	static const std::ostringstream& GetSpdOutput();
	static void ClearBuffer();

private:
	static std::shared_ptr<spdlog::logger> s_engineLogger;
	static std::shared_ptr<spdlog::logger> s_applicationLogger;

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
#define CD_ENGINE_ASSERT(x, ...) { if(!(x)) { ::engine::Log::GetEngineLogger()->error(__VA_ARGS__); } }
#define CD_ASSERT(x, ...) { if(!(x)) { ::engine::Log::GetApplicationLogger()->error(__VA_ARGS__); } }

inline std::ostream& operator<<(std::ostream& os, const cd::Vec2f& vec)
{
	return os << std::format("({0}, {1})", vec.x(), vec.y());
}

inline std::ostream& operator<<(std::ostream& os, const cd::Vec3f& vec)
{
	return os << std::format("({0}, {1}, {2})", vec.x(), vec.y(), vec.z());
}

inline std::ostream& operator<<(std::ostream& os, const cd::Vec4f& vec)
{
	return os << std::format("({0}, {1}, {2}, {3})", vec.x(), vec.y(), vec.z(), vec.w());
}

inline std::ostream& operator<<(std::ostream& os, const cd::Quaternion& quaternion)
{
	return os << std::format("Vector = ({0}, {1}, {2}), Scalar = {3}", quaternion.x(), quaternion.y(), quaternion.z(), quaternion.w());
}

#else

class Log
{
public:
	static void Init() {}
};

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