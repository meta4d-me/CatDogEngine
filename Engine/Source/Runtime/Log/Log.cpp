#include "Log.h"

#ifdef ENABLE_SPDLOG
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/ostream_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace engine
{

std::shared_ptr<spdlog::logger>& Log::GetEngineLogger()
{
	return s_engineLogger;
}

std::shared_ptr<spdlog::logger>& Log::GetApplicationLogger()
{
	return s_applicationLogger;
}

const std::ostringstream& Log::GetSpdOutput()
{
	return m_oss;
}

void Log::ClearBuffer()
{
	m_oss.str("");
}

void Log::Init() {
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_pattern("%^[%T] %n: %v%$");

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("CatDog.log", true);
	fileSink->set_pattern("[%T] [%n] [%l]: %v");

	auto ossSink = std::make_shared<spdlog::sinks::ostream_sink_mt>(m_oss);
	ossSink->set_pattern("[%T] [%n] [%l]: %v");

	std::vector<spdlog::sink_ptr> sinks{ consoleSink, fileSink, ossSink };

	s_engineLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
	spdlog::register_logger(s_engineLogger);
	s_engineLogger->set_level(spdlog::level::trace);
	s_engineLogger->flush_on(spdlog::level::trace);

	s_applicationLogger = std::make_shared<spdlog::logger>("EDITOR", sinks.begin(), sinks.end());
	spdlog::register_logger(s_applicationLogger);
	s_applicationLogger->set_level(spdlog::level::trace);
	s_applicationLogger->flush_on(spdlog::level::trace);
}

}

#endif