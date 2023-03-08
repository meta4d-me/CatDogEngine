#pragma once

#include<string>

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

static Language GetLanguageFromName(const std::string& name)
{
	for (int i = 0; i < static_cast<int>(Language::Count); ++i)
	{
		if (name == LanguageNames[i])
		{
			return static_cast<Language>(i);
		}
	}
	// If not found retrun default Language:English
	return Language::English;
}


}