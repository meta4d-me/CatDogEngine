#pragma once

#include "Core/StringCrc.h"

#include <string>

namespace engine
{

class NameComponent
{
public:
	static constexpr StringCrc GetClassName()
	{
		constexpr StringCrc className("NameComponent");
		return className;
	}

public:
	NameComponent() = default;
	NameComponent(const NameComponent&) = default;
	NameComponent& operator=(const NameComponent&) = default;
	NameComponent(NameComponent&&) = default;
	NameComponent& operator=(NameComponent&&) = default;
	~NameComponent() = default;

	void SetName(std::string name);
	const char* GetName() const { return m_name.c_str(); }
	StringCrc GetNameCrc() const { return m_nameCrc; }

	bool operator==(const NameComponent& other) const { return m_nameCrc == other.m_nameCrc; }
	bool operator!=(const NameComponent& other) const { return m_nameCrc != other.m_nameCrc; }

private:
	// Input
	std::string m_name;

	// Output
	StringCrc m_nameCrc;
};

}