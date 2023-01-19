#include "Process.h"

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

	subprocess_create_ex(commandLine.data(), 0, environments.data(), m_pProcess.get());

	// LOG
	printf("Start process %s\n", m_processName.c_str());
	for(int i = 1, num = static_cast<int>(commandLine.size() - 1); i < num; ++i)
	{
		printf("\tArgument %s\n", commandLine[i]);
	}
	for (int i = 1, num = static_cast<int>(environments.size() - 1); i < num; ++i)
	{
		printf("\tEnvironment %s\n", environments[i]);
	}

	if (m_waitUntilFinished)
	{
		int processResult;
		subprocess_join(m_pProcess.get(), &processResult);
	}
	printf("End process %s\n", m_processName.c_str());
}

}