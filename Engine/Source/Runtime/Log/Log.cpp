#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace engine
{

std::shared_ptr<spdlog::logger> Log::s_engineLogger;
std::shared_ptr<spdlog::logger> Log::s_applicationLogger;

void Log::Init() {
	auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	consoleSink->set_pattern("%^[%T] %n: %v%$");

	auto fileSink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("CatDog.log", true);
	fileSink->set_pattern("[%T] [%l] %n: %v");

	std::vector<spdlog::sink_ptr> sinks{ consoleSink , fileSink };

	s_engineLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
	spdlog::register_logger(s_engineLogger);
	s_engineLogger->set_level(spdlog::level::trace);
	s_engineLogger->flush_on(spdlog::level::trace);

	s_applicationLogger = std::make_shared<spdlog::logger>("APP", sinks.begin(), sinks.end());
	spdlog::register_logger(s_applicationLogger);
	s_applicationLogger->set_level(spdlog::level::trace);
	s_applicationLogger->flush_on(spdlog::level::trace);
}

} // namespace engine