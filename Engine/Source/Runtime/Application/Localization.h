#pragma once

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
void Read_csv();
const char* SetText(const char* text);

}