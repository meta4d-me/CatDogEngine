#pragma once

#include "ini.h"
#include "Language.h"

#include <filesystem>
#include <string>


namespace engine
{

class UserSetting
{
public:
	static Language LoadLanguageSetting();
	static void SaveLanguageSetting(Language language);

private:
    static std::string iniPath;
	static std::string Get_ini_Path();
};
}