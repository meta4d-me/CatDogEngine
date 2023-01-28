#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace engine
{

std::shared_ptr<spdlog::logger> Log::s_engineLogger;
std::shared_ptr<spdlog::logger> Log::s_editorLogger;

void Log::Init() {
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_pattern("%^[%T] %n: %v%$");

	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("CatDog.log", true);
	file_sink->set_pattern("[%T] [%l] %n: %v");

	std::vector<spdlog::sink_ptr> sinks{ console_sink , file_sink };

	s_engineLogger = std::make_shared<spdlog::logger>("ENGINE", sinks.begin(), sinks.end());
	spdlog::register_logger(s_engineLogger);
	s_engineLogger->set_level(spdlog::level::trace);
	s_engineLogger->flush_on(spdlog::level::trace);

	s_editorLogger = std::make_shared<spdlog::logger>("EDITOR", sinks.begin(), sinks.end());
	spdlog::register_logger(s_editorLogger);
	s_editorLogger->set_level(spdlog::level::trace);
	s_editorLogger->flush_on(spdlog::level::trace);
}

} // namespace engine