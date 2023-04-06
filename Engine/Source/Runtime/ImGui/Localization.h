#pragma once

#include "Language.h"

#include <fstream>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace engine
{

class Localization
{
public:
    static void SetLanguage(Language language);
    static const char* GetText(const std::string& key);

private:
    static void ReadCSV();

private:
    static std::map<std::string, std::vector<std::string>> TextMap;
    static Language m_language;
};

#define CD_TEXT(TEXT_KEY) engine::Localization::GetText((TEXT_KEY))

}

