#pragma once

namespace engine::Hasher
{

namespace details
{

template<std::size_t size>
struct fnv1a_traits;

template<>
struct fnv1a_traits<4>
{
	using type = std::uint32_t;
	static constexpr type offset = 2166136261U;
	static constexpr type prime = 16777619U;
};

template<>
struct fnv1a_traits<8>
{
	using type = std::uint64_t;
	static constexpr type offset = 14695981039346656037ull;
	static constexpr type prime = 1099511628211ull;
};

}

constexpr std::size_t string_hash_seed(std::size_t seed, const char* str, std::size_t n) noexcept
{
	using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
	
	std::size_t value = seed;
	for (std::size_t i = 0; i < n; ++i)
	{
		value = (value ^ static_cast<Traits::type>(str[i])) * Traits::prime;
	}

	return value;
}

constexpr std::size_t string_hash(const char* str, std::size_t n) noexcept
{
	using Traits = details::fnv1a_traits<sizeof(std::size_t)>;
	return string_hash_seed(Traits::offset, str, n);
}

}