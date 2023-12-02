#pragma once

#include "Delegate.hpp"

#include <list>

namespace engine
{

template<typename T>
class MulticastDelegate
{
private:
	static_assert("MulticastDelegate class should accept more than one parameter. Void should also pass through.");
};

template<typename RetVal, typename... Args>
class MulticastDelegate<RetVal(Args...)>
{
public:
	MulticastDelegate() = default;
	MulticastDelegate(const MulticastDelegate&) = delete;
	MulticastDelegate& operator=(const MulticastDelegate&) = delete;
	MulticastDelegate(MulticastDelegate&&) = default;
	MulticastDelegate& operator=(MulticastDelegate&&) = default;
	~MulticastDelegate() = default;

	template<RetVal(*Function)(Args...)>
	void Bind()
	{
		Delegate<RetVal(Args...)> delegate;
		delegate.template Bind<Function>();
		m_delegates.emplace_back(std::move(delegate));
	}

	template<typename C, RetVal(C::*Function)(Args...)>
	void Bind(C* pInstance)
	{
		Delegate<RetVal(Args...)> delegate;
		delegate.template Bind<C, Function>(pInstance);
		m_delegates.emplace_back(std::move(delegate));
	}

	template<typename C, RetVal(C::*Function)(Args...) const>
	void Bind(const C* pInstance)
	{
		Delegate<RetVal(Args...)> delegate;
		delegate.template Bind<C, Function>(pInstance);
		m_delegates.emplace_back(std::move(delegate));
	}

	bool Empty() const { return m_delegates.empty(); }

	void Invoke(Args... args) const
	{
		for (const auto& delegate : m_delegates)
		{
			delegate.Invoke(args...);
		}
	}

private:
	std::list<Delegate<RetVal(Args...)>> m_delegates;
};

}