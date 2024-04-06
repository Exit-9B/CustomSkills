#pragma once

#include "CustomSkills/Events.h"

namespace CustomSkills
{
	class SkillIncreaseEventSource : public RE::BSTEventSource<SkillIncreaseEvent>
	{
	public:
		static SkillIncreaseEventSource* Instance();

		SkillIncreaseEventSource(const SkillIncreaseEventSource&) = delete;
		SkillIncreaseEventSource(SkillIncreaseEventSource&&) = delete;

		~SkillIncreaseEventSource() = default;

		SkillIncreaseEventSource& operator=(const SkillIncreaseEventSource&) = delete;
		SkillIncreaseEventSource& operator=(SkillIncreaseEventSource&&) = delete;

	private:
		SkillIncreaseEventSource() = default;
	};
}
