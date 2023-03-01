#include "Localization.h"

namespace engine
{// Define global variables
    Language m_language = Language::English;//m_lang should be changed when choosing the language type
   std::map<std::string, std::vector<std::string>> Text_Map;
  

    // Read data from CSV file and return as a map
    void ReadCSV(std::string csv_path) {
        std::string CSVPath = CDEDITOR_RESOURCES_ROOT_PATH;
        std::string FileName = "Text.csv";
        CSVPath += FileName;
        std::ifstream file(CSVPath);
        if (!file.is_open()) {
            std::cout << "Failed to open CSV file: " << csv_path << std::endl;
            return;
        }
        std::string line;
        while (std::getline(file, line)) {
            std::string::size_type pos = 0, last_pos = 0;
            std::vector<std::string> words;
            words.reserve(3); // key-CN-EN
            for (char& c : line) {
                if (c == ',') {
                    words.emplace_back(std::move(line.substr(last_pos, pos - last_pos)));
                    last_pos = pos + 1;
                }
                ++pos;
            }
            words.emplace_back(std::move(line.substr(last_pos)));
            Text_Map[std::move(words[0])] = { std::move(words[1]), std::move(words[2]) };
        }
    }

      const char* SetText(std::string key) {
         if(Text_Map.empty())
        ReadCSV("D:\\code\\Text1.csv");//it should join the initialization phase
        auto itKeyValue = Text_Map.find(key);
        if (itKeyValue != Text_Map.end()) {
            if (m_language == Language::ChineseSimplied) {
                return itKeyValue->second[0].c_str();
            }
            else if (m_language == Language::English) {
                return itKeyValue->second[1].c_str();
            }
        }
        return "Not Found";
    }

}
