#pragma once

#ifdef ENABLE_SUBPROCESS
#include "Base/Template.h"
#include "Process/subprocess.h"

#include <memory>
#include <string>
#include <vector>

namespace editor
{

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

	void SetPrintChildProcessLog(bool doPrint) { m_printChildProcessLog = doPrint; }
	void SetWaitUntilFinished(bool doWait) { m_waitUntilFinished = doWait; }
	void SetCommandArguments(std::vector<std::string> arguments) { m_commandArguments = cd::MoveTemp(arguments); }
	void SetEnvironments(std::vector<std::string> environments) { m_environments = cd::MoveTemp(environments); }
	
	void* GetProcessHandle() const;
	bool IsAlive() const;
	bool IsWaitUntilFinished() const { return m_waitUntilFinished; }
	void Run();

private:
	std::unique_ptr<subprocess_s> m_pProcess;

	std::string m_processName;
	std::vector<std::string> m_commandArguments;
	std::vector<std::string> m_environments;
	bool m_waitUntilFinished = false;
	bool m_printChildProcessLog = true;
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

	void* GetProcessHandle() const { return nullptr; }
	bool IsAlive() const { return false; }
	bool IsWaitUntilFinished() const { return false; }
	void Run() {}
};

}

#endif