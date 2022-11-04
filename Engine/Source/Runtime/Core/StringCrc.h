#pragma once

#include "Hashers/StringHash.hpp"

namespace engine
{

class StringCrc final
{
public:
	StringCrc() = default;
	explicit constexpr StringCrc(std::string_view sv) : m_hashValue(Hasher::string_hash(sv)) {}
	explicit constexpr StringCrc(const char* str, std::size_t n) : m_hashValue(Hasher::string_hash(str, n)) {}
	StringCrc(const StringCrc&) = default;
	StringCrc& operator=(const StringCrc&) = default;
	StringCrc(StringCrc&&) = default;
	StringCrc& operator=(StringCrc&&) = default;
	~StringCrc() = default;

	constexpr size_t value() const { return m_hashValue; }
	bool operator==(const StringCrc& other) const { return m_hashValue == other.m_hashValue; }
	bool operator!=(const StringCrc& other) const { return m_hashValue != other.m_hashValue; }

private:
	size_t m_hashValue;
};

}