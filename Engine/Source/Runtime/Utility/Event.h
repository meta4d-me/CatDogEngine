#pragma once

#include <functional>

namespace engine::Tools
{
	using ListenerID = uint64_t;

	template<class... ArgTypes>
	class Event
	{
	public:
		using Callback = std::function<void(ArgTypes...)>;

		bool operator-=(ListenerID p_listenerID);

		ListenerID operator+=(Callback p_callback);

		ListenerID AddListener(Callback p_callback);

		bool RemoveListener(ListenerID p_listenerID);

		void RemoveAllListeners();

		uint64_t GetListenerCount();

		void Invoke(ArgTypes... p_args);

	private:
		std::unordered_map<ListenerID, Callback>	m_callbacks;
		ListenerID									m_availableListenerID = 0;
	};

	template<class... ArgTypes>
	ListenerID Event<ArgTypes...>::AddListener(Callback p_callback)
	{
		ListenerID listenerID = m_availableListenerID++;
		m_callbacks.emplace(listenerID, p_callback);
		return listenerID;
	}

	template<class... ArgTypes>
	ListenerID Event<ArgTypes...>::operator+=(Callback p_callback)
	{
		return AddListener(p_callback);
	}

	template<class... ArgTypes>
	bool Event<ArgTypes...>::RemoveListener(ListenerID p_listenerID)
	{
		return m_callbacks.erase(p_listenerID) != 0;
	}

	template<class... ArgTypes>
	bool Event<ArgTypes...>::operator-=(ListenerID p_listenerID)
	{
		return RemoveListener(p_listenerID);
	}

	template<class... ArgTypes>
	void Event<ArgTypes...>::RemoveAllListeners()
	{
		m_callbacks.clear();
	}

	template<class... ArgTypes>
	uint64_t Event<ArgTypes...>::GetListenerCount()
	{
		return m_callbacks.size();
	}

	template<class... ArgTypes>
	void Event<ArgTypes...>::Invoke(ArgTypes... p_args)
	{
		for (auto const& [key, value] : m_callbacks)
			value(p_args...);
	}
}