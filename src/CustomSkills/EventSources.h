#pragma once

#include "CustomSkills/Events.h"

namespace CustomSkills
{
	template <class Event>
	class EventSourceSingleton : public RE::BSTEventSource<Event>
	{
	public:
		static RE::BSTEventSource<Event>* Get()
		{
			static EventSourceSingleton<Event> instance{};
			return &instance;
		}

		EventSourceSingleton(const EventSourceSingleton&) = delete;
		EventSourceSingleton(EventSourceSingleton&&) = delete;

		~EventSourceSingleton() = default;

		EventSourceSingleton& operator=(const EventSourceSingleton&) = delete;
		EventSourceSingleton& operator=(EventSourceSingleton&&) = delete;

	private:
		EventSourceSingleton() = default;
	};

	using SkillIncreaseEventSource = EventSourceSingleton<SkillIncreaseEvent>;
}
