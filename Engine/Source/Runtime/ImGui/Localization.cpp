#include "Localization.h"

#include <Log/log.h>

namespace engine
{
std::map<std::string, std::vector<std::string>> Localization::TextMap;
Language Localization::m_language = UserSetting::LoadLanguageSetting();

void Localization::ChangeLanguage(Language language)
{
    m_language = language;
}

const char* Localization::GetText(const std::string& key)
{
    if (TextMap.empty())
    Localization::ReadCSV();
    
    auto itKeyValue = TextMap.find(key);
    if (itKeyValue != TextMap.end())
    {
        if ( m_language == Language::ChineseSimplied)
        {
            return itKeyValue->second[0].c_str();
        }
        else if (m_language == Language::English)
        {
            return itKeyValue->second[1].c_str();
        }
    }

    return "Not Found";
}

void Localization::ReadCSV()
{
    std::filesystem::path rootPath = CDEDITOR_RESOURCES_ROOT_PATH;
    std::string CSVPath = (rootPath / "Text.csv").string();
    std::ifstream file(std::move(CSVPath));
    if (!file.is_open())
    {
        CD_ERROR("Failed to open CSV file");
        return;
    }

    std::string line;
    std::getline(file, line);
    int num_words = 0;
    for (char c : line)
    {
        if (c == ',') 
        {

            num_words++;

        }
    }
    num_words++;

    while (std::getline(file, line))
    {
        std::string::size_type pos = 0, last_pos = 0;
        std::vector<std::string> words;
        words.reserve(num_words); 
        for (char& c : line)
        {
            if (c == ',')
            {
                words.push_back(line.substr(last_pos, pos - last_pos));
                last_pos = pos + 1;
            }
            ++pos;
        }
        words.push_back(line.substr(last_pos));
        TextMap[std::move(words[0])] = { std::move(words[1]), std::move(words[2]) };
    }
}

}
