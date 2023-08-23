#include "Localization.h"

#include "Base/Template.h"

#include <fstream>

namespace engine
{

Language Localization::m_language = Language::English;
std::map<std::string, std::vector<std::string>> Localization::TextMap;

void Localization::SetLanguage(Language language)
{
     m_language = language;
}

const char* Localization::GetText(const std::string& key)
{
    if (!TextMap.empty())
    {
        auto itKeyValue = TextMap.find(key);
        if (itKeyValue != TextMap.end())
        {
            if (m_language == Language::ChineseSimplied)
            {
                return itKeyValue->second[0].c_str();
            }
            else if (m_language == Language::English)
            {
                return itKeyValue->second[1].c_str();
            }
        }
    }

    return key.c_str();
}

bool Localization::ReadCSV(std::string filePath)
{
    std::ifstream fin(filePath);
    if (!fin.is_open())
    {
        return false;
    }

    std::string line;
    std::getline(fin, line);
    int numWords = 0;
    for (char c : line)
    {
        if (',' == c)
        {
            ++numWords;
        }
    }
    ++numWords;

    while (std::getline(fin, line))
    {
        std::string::size_type pos = 0, last_pos = 0;
        std::vector<std::string> words;
        words.reserve(numWords);
        for (char c : line)
        {
            if (c == ',')
            {
                words.push_back(line.substr(last_pos, pos - last_pos));
                last_pos = pos + 1;
            }
            ++pos;
        }
        words.push_back(line.substr(last_pos));
        TextMap[cd::MoveTemp(words[0])] = { cd::MoveTemp(words[1]), cd::MoveTemp(words[2]) };
    }

    fin.close();

    return true;
}

}
