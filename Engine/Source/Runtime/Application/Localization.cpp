#include<sstream>
#include<string>
#include <vector>
#include<numeric>
#include<filesystem>
#include<map>
#include<utility>
#include<fstream>
#include <iostream>
#include "Localization.h"

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
