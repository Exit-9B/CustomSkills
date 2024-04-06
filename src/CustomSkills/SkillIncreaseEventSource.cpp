#include "SkillIncreaseEventSource.h"

namespace CustomSkills
{
	SkillIncreaseEventSource* SkillIncreaseEventSource::Instance()
	{
		static SkillIncreaseEventSource instance{};
		return &instance;
	}
}
