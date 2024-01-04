#pragma once

#include <cassert>
#include <functional>

namespace engine
{

template<typename T>
class Delegate
{
private:
	static_assert("Delegate class should accept more than one parameter. Void should also pass through.");
};

template<typename RetVal, typename... Args>
class Delegate<RetVal(Args...)>
{
private:
	// The ProxyFunction is a function pointer which keeps a same signature
	// for different kinds of function calls.
	using ProxyFuncPtr = RetVal(*)(void*, Args...);

	// Basic functions
	template<RetVal(*Function)(Args...)>
	static RetVal FunctionProxy(void*, Args... args)
	{
		return Function(std::forward<Args>(args)...);
	}

	// Class's non-const methods
	template<typename C, RetVal(C::*Function)(Args...)>
	static RetVal MethodProxy(void* pInstance, Args... args)
	{
		return (static_cast<C*>(pInstance)->*Function)(std::forward<Args>(args)...);
	}

	// Class's const methods
	template<typename C, RetVal(C::*Function)(Args...) const>
	static RetVal ConstMethodProxy(void* pInstance, Args... args)
	{
		return (static_cast<const C*>(pInstance)->*Function)(std::forward<Args>(args)...);
	}

public:
	Delegate() = default;
	Delegate(const Delegate&) = default;
	Delegate& operator=(const Delegate&) = default;
	Delegate(Delegate&&) = default;
	Delegate& operator=(Delegate&&) = default;
	~Delegate() = default;

	template<RetVal(*Function)(Args...)>
	void Bind()
	{
		m_pInstance = nullptr;
		m_pProxyFunc = &FunctionProxy<Function>;
	}

	template<typename C, RetVal(C::*Function)(Args...)>
	void Bind(C* pInstance)
	{
		m_pInstance = pInstance;
		m_pProxyFunc = &MethodProxy<C, Function>;
	}

	template<typename C, RetVal(C::*Function)(Args...) const>
	void Bind(const C* pInstance)
	{
		m_pInstance = const_cast<C*>(pInstance);
		m_pProxyFunc = &ConstMethodProxy<C, Function>;
	}

	bool Empty() const { return m_pProxyFunc == nullptr; }

	RetVal Invoke(Args... args) const
	{
		//assert(m_pProxyFunc != nullptr && "Cannot invoke unbound Delegate. Call Bind() first.");
		if constexpr (std::is_same_v<RetVal, void>)
		{
			if (m_pProxyFunc != nullptr)
			{
				m_pProxyFunc(m_pInstance, std::forward<Args>(args)...);
			}
		}
		else
		{
			return m_pProxyFunc != nullptr ? m_pProxyFunc(m_pInstance, std::forward<Args>(args)...) : RetVal{};
		}
	}

private:
	void* m_pInstance = nullptr;
	ProxyFuncPtr m_pProxyFunc = nullptr;
};

}