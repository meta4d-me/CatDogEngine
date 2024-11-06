#include "Log.h"

#ifdef SPDLOG_ENABLE

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

namespace engine
{

std::shared_ptr<spdlog::logger> Log::s_pEngineLogger;
std::shared_ptr<spdlog::logger> Log::s_pApplicationLogger;
std::ostringstream Log::m_oss;

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

	s_pEngineLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
	spdlog::register_logger(s_pEngineLogger);
	s_pEngineLogger->set_level(spdlog::level::trace);
	s_pEngineLogger->flush_on(spdlog::level::trace);

	s_pApplicationLogger = std::make_shared<spdlog::logger>("EDITOR", sinks.begin(), sinks.end());
	spdlog::register_logger(s_pApplicationLogger);
	s_pApplicationLogger->set_level(spdlog::level::trace);
	s_pApplicationLogger->flush_on(spdlog::level::trace);
}

}

#endif