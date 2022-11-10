#pragma once

#include "Hashers/StringHash.hpp"

namespace engine
{

class StringCrc final
{
public:
	StringCrc() = default;
	explicit constexpr StringCrc(std::string_view sv) : m_hashValue(cdtools::StringHash<uint32_t>(sv)) {}
	explicit constexpr StringCrc(const char* str, std::size_t n) : m_hashValue(cdtools::StringHash<uint32_t>(str, n)) {}
	StringCrc(const StringCrc&) = default;
	StringCrc& operator=(const StringCrc&) = default;
	StringCrc(StringCrc&&) = default;
	StringCrc& operator=(StringCrc&&) = default;
	~StringCrc() = default;

	constexpr uint32_t value() const { return m_hashValue; }
	bool operator==(const StringCrc& other) const { return m_hashValue == other.m_hashValue; }
	bool operator!=(const StringCrc& other) const { return m_hashValue != other.m_hashValue; }

private:
	uint32_t m_hashValue;
};

}