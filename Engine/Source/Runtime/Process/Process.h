#pragma once

#ifdef ENABLE_SUBPROCESS
#include "Base/Template.h"
#include "Core/Delegates/Delegate.hpp"
#include "Process/subprocess.h"

#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace editor
{

enum class OutputType
{
	StdOut,
	StdErr,
};

class Process final
{
public:
	Process() = delete;
	explicit Process(const char* pProcessName);
	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;
	Process(Process&&) = default;
	Process& operator=(Process&&) = default;
	~Process();

	void SetHandle(uint32_t handle) { m_handle = handle; }
	void SetPrintChildProcessLog(bool doPrint) { m_printChildProcessLog = doPrint; }
	void SetPrintChildProcessErrorLog(bool doPrint) { m_printChildProcessErrorLog = doPrint; }
	void SetWaitUntilFinished(bool doWait) { m_waitUntilFinished = doWait; }
	void SetCommandArguments(std::vector<std::string> arguments) { m_commandArguments = cd::MoveTemp(arguments); }
	void SetEnvironments(std::vector<std::string> environments) { m_environments = cd::MoveTemp(environments); }
	void Run();

	engine::Delegate<void(uint32_t handle, std::span<const char> str)> m_onOutput;
	engine::Delegate<void(uint32_t handle, std::span<const char> str)> m_onErrorOutput;

private:
	using SubProcessReadLogFunction = unsigned (*)(struct subprocess_s* const, char* const, unsigned);
	void PrintSubProcessLog(OutputType outputType, subprocess_s* const pSubProcess, SubProcessReadLogFunction readMethod);

	std::unique_ptr<subprocess_s> m_pProcess;
	uint32_t m_handle;

	std::string m_processName;
	std::vector<std::string> m_commandArguments;
	std::vector<std::string> m_environments;
	bool m_waitUntilFinished = false;

	bool m_printChildProcessLog = false;
	bool m_printChildProcessErrorLog = true;
};

}

#else

#include "Base/Template.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

class Process final
{
public:
	Process() = delete;
	explicit Process(const char*) {}
	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;
	Process(Process&&) = default;
	Process& operator=(Process&&) = default;
	~Process() = default;

	void SetPrintChildProcessLog(bool doPrint) {}
	void SetWaitUntilFinished(bool doWait) {}
	void SetCommandArguments(std::vector<std::string> arguments) {}
	void SetEnvironments(std::vector<std::string> environments) {}
	void Run() {}
};

}

#endif