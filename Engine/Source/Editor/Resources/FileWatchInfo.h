#pragma once

#include "Core/Delegates/Delegate.hpp"

#include <string>

namespace editor
{

class FileWatchInfo final
{
public:
	FileWatchInfo() = default;
	FileWatchInfo(const FileWatchInfo&) = default;
	FileWatchInfo& operator=( const FileWatchInfo&) = default;
	FileWatchInfo(FileWatchInfo&&) = default;
	FileWatchInfo& operator=(FileWatchInfo&&) = default;
	~FileWatchInfo() = default;

	std::string m_watchPath = "";
	bool m_isrecursive = false;

	engine::Delegate<void(const char* rootDir, const char* filePath)> m_onCreate;
	engine::Delegate<void(const char* rootDir, const char* filePath)> m_onDelete;
	engine::Delegate<void(const char* rootDir, const char* filePath)> m_onModify;
	engine::Delegate<void(const char* rootDir, const char* filePath, const char* oldFilePath)> m_onMove;
};

}