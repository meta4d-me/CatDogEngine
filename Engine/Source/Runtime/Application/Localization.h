#pragma once
#include<sstream>
#include<string>
#include <vector>
#include<numeric>
#include<filesystem>
#include<map>
#include<utility>
#include<fstream>
#include <iostream>



namespace engine
{

enum class Language
{
	ChineseSimplied,
	ChineseTraditional,
	Cyrillic,
	English,
	Greek,
	Japanese,
	Korean,
	Thai,
	Vitnam,
	Count,
};
static constexpr const char* LanguageNames[] =
{
	"ChineseSimplied",
	"ChineseTraditional",
	"Cyrillic",
	"English",
	"Greek",
	"Japanese",
	"Korean",
	"Thai",
	"Vitnam"
};
// Sanity check for enum and name mapping.
static_assert(static_cast<int>(Language::Count) == sizeof(LanguageNames) / sizeof(char*));

static constexpr const char* GetLanguageName(Language language)
{
	return LanguageNames[static_cast<int>(language)];
}
<<<<<<< HEAD

const char* SetText(std::string key);
void ReadCSV(std::string csv_path);

=======
//void ReadCSV(std::string csv_path);
//const char* SetText(std::string key);
>>>>>>> 651486cfc86c1e28e6d4b7f78cfdff56571ef960

}
