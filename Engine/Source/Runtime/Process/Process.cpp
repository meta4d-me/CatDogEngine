#ifdef ENABLE_SUBPROCESS

#include "Process.h"

#include "Base/NameOf.h"
#include "Log/Log.h"

namespace editor
{

Process::Process(const char* pProcessName) :
	m_processName(pProcessName)
{
}

Process::~Process()
{
	if (m_pProcess)
	{
		subprocess_destroy(m_pProcess.get());
	}
}

void Process::Run()
{
	m_pProcess = std::make_unique<subprocess_s>();

	std::vector<const char*> commandLine;
	commandLine.push_back(m_processName.c_str());
	for (const std::string& commandArgument : m_commandArguments)
	{
		commandLine.push_back(commandArgument.c_str());
	}
	commandLine.push_back(nullptr); // subprocess api doesn't accept commandCount so we need to use nullptr as a terminate symbol.

	std::vector<const char*> environments;
	for (const std::string& environment : m_environments)
	{
		environments.push_back(environment.c_str());
	}
	environments.push_back(nullptr);

	int processOptions = subprocess_option_combined_stdout_stderr | subprocess_option_no_window | subprocess_option_enable_async;
	subprocess_create_ex(commandLine.data(), processOptions, environments.data(), m_pProcess.get());

	// LOG
	CD_ENGINE_INFO("Start process {0}", m_processName.c_str());

	for(int i = 1, num = static_cast<int>(commandLine.size() - 1); i < num; ++i)
	{
		CD_ENGINE_TRACE("\tArgument {0}", commandLine[i]);
	}
	for (int i = 1, num = static_cast<int>(environments.size() - 1); i < num; ++i)
	{
		CD_ENGINE_TRACE("\tEnvironment {0}", environments[i]);
	}

	if (m_printChildProcessLog)
	{
		PrintSubProcessLog(OutputType::StdOut, m_pProcess.get(), subprocess_read_stdout);
	}

	if (m_printChildProcessErrorLog)
	{
		PrintSubProcessLog(OutputType::StdErr, m_pProcess.get(), subprocess_read_stderr);
	}

	if (m_waitUntilFinished)
	{
		int processResult;
		subprocess_join(m_pProcess.get(), &processResult);
		CD_ENGINE_INFO("End process {0}", m_processName.c_str());
	}
}

void Process::PrintSubProcessLog(OutputType outputType, subprocess_s* const pSubProcess, SubProcessReadLogFunction readMethod)
{
	static char processOutputData[65536] = { 0 };
	uint32_t processOutputDataIndex = 0U;
	uint32_t processOutputDataReadBytes = 0U;

	do
	{
		processOutputDataReadBytes = readMethod(pSubProcess, processOutputData + processOutputDataIndex, sizeof(processOutputData) - 1 - processOutputDataIndex);
		processOutputDataIndex += processOutputDataReadBytes;
	}
	while (processOutputDataReadBytes != 0U);

	if (processOutputDataIndex > 0U)
	{
		// Logs from child process's stdout maybe error info because many tool authors will use stdout to print rather than stderr.
		if (OutputType::StdOut == outputType)
		{
			CD_ENGINE_TRACE("{0}\n{1}", nameof::nameof_enum(OutputType::StdOut), processOutputData);
			m_onOutput.Invoke(m_handle, processOutputData);
		}
		else if (OutputType::StdErr == outputType)
		{
			CD_ENGINE_ERROR("{0}\n{1}", nameof::nameof_enum(OutputType::StdErr), processOutputData);
			m_onErrorOutput.Invoke(m_handle, processOutputData);
		}
	}
}

}

#endif