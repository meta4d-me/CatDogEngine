#include "Localization.h"
<<<<<<< HEAD
namespace engine
{// Define global variables
    Language m_lang = Language::ChineseSimplied;//m_lang should be changed when choosing the language type
    std::map<std::string, std::vector<std::string>> Text_Map;
  

    // Read data from CSV file and return as a map
    void ReadCSV(std::string csv_path) {
        std::ifstream file(csv_path);
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
        ReadCSV("D:\\code\\Text.csv");//it should join the initialization phase
        auto itKeyValue = Text_Map.find(key);
        if (itKeyValue != Text_Map.end()) {
            if (m_lang == Language::ChineseSimplied) {
                return itKeyValue->second[0].c_str();
            }
            else if (m_lang == Language::English) {
                return itKeyValue->second[1].c_str();
            }
        }
        return "Not Found";
    }

}
=======

//namespace engine
//{// Define global variables
//    std::map<std::string, std::vector<std::string>> Text_Map;
//    
//
//    // Read data from CSV file and return as a map
//    void ReadCSV(std::string csv_path) {
//        std::ifstream file(csv_path);
//        if (!file.is_open()) {
//            std::cout << "Failed to open CSV file: " << csv_path << std::endl;
//            return;
//        }
//        std::string line;
//        while (getline(file, line)) {
//            std::istringstream ss(line);
//            std::vector<std::string> words;
//            std::string word;
//            while (getline(ss, word, ',')) {
//                words.push_back(word);
//            }
//            std::string key = words[0];
//            std::string chinese = words[1];
//            std::string english = words[2];
//            Text_Map[key] = { chinese, english };
//        }
//    }
//
//    const char* SetText(std::string key) {
//        if (Text_Map.find(key) != Text_Map.end()) {
//            if (1) {
//                return Text_Map[key][0].c_str();
//            }
//            else if (0) {
//                return Text_Map[key][1].c_str();
//            }
//        }
//        return "Not Found";
//    }
//
//}
>>>>>>> 651486cfc86c1e28e6d4b7f78cfdff56571ef960
