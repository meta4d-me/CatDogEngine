#include "UserSetting.h"

namespace engine
{
std::string UserSetting::iniPath = UserSetting::Get_ini_Path();

std::string UserSetting::Get_ini_Path()
{
	std::filesystem::path rootPath = CDEDITOR_RESOURCES_ROOT_PATH;
	std::string INIPath = (rootPath / "EditorUserSettings.ini").string();
	return std::move(INIPath);
}

Language UserSetting::LoadLanguageSetting()
{
	mINI::INIFile file(Get_ini_Path());
	mINI::INIStructure ini;
	file.read(ini);
	std::string& typeOfLanguages = ini["menu_interface"]["Languages"];
	return GetLanguageFromName(typeOfLanguages);
}

void UserSetting::SaveLanguageSetting(Language language)
{
	const char* str_language = GetLanguageName(language);
	mINI::INIFile file(Get_ini_Path());
	mINI::INIStructure ini;
	file.read(ini);
	ini["menu_interface"]["Languages"] = str_language;
	file.write(ini);

}

}