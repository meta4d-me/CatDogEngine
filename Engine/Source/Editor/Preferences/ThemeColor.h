#pragma once

namespace editor
{

enum class ThemeColor
{
	Black = 0,
	Classic,
	Dark,
	Grey,
	Light,
	Count
};

static constexpr const char* ThemeColorNames[] =
{
	"Black",
	"Classic",
	"Dark",
	"Grey",
	"Light",
};

// Sanity check for enum and name mapping.
static_assert(static_cast<int>(ThemeColor::Count) == sizeof(ThemeColorNames) / sizeof(char*));

static constexpr const char* GetThemeColorName(ThemeColor theme)
{
	return ThemeColorNames[static_cast<int>(theme)];
}

}