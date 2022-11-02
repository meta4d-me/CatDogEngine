#pragma once

#include <string_view>

namespace engine::Hasher
{

constexpr std::size_t string_hash_seed(std::size_t seed, const char* str, std::size_t N) noexcept;

constexpr std::size_t string_hash(const char* str, std::size_t n) noexcept;
constexpr std::size_t string_hash(std::string_view sv) noexcept { return string_hash(sv.data(), sv.size()); }

}

#include "StringHash.inl"