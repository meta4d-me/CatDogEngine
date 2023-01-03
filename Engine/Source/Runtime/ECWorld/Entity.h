#pragma once

//#include <compare> // operator<=>

#include <inttypes.h>

namespace engine
{

// Entity is a global unique unsigned integer identifier [0, max - 1] in the engine runtime.
using Entity = uint32_t;
static constexpr Entity INVALID_ENTITY = static_cast<uint32_t>(-1);

// A wrapper of u32 instead of using u32:
// 1.Not sure if I should more data fields to Entity class
// 2.Trivial class should be fast nearly zero overhead.
// 3.Type safe in actual work.
//class Entity
//{
//public:
//	using EntityID = std::uint32_t;
//	static constexpr EntityID INVALID_ENTITY = static_cast<std::uint32_t>(-1);
//
//public:
//	Entity() = default;
//	explicit Entity(EntityID id) : m_id(id) {}
//	Entity(const Entity&) = default;
//	Entity& operator=(const Entity&) = default;
//	Entity(Entity&&) = default;
//	Entity& operator=(Entity&&) = default;
//	~Entity() = default;
//
//	constexpr bool IsValid() const { return m_id != INVALID_ENTITY; }
//	constexpr EntityID GetID() const { return m_id; }
//
//	// Defaulting <=> automatically gives ==, !=, <, >, <=, >= for free.
//	constexpr auto operator<=>(const Entity&) const = default;
//
//private:
//	EntityID m_id;
//};
//
//static_assert(std::is_standard_layout_v<Entity> && std::is_trivial_v<Entity>);

}