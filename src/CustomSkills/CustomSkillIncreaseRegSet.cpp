#include "CustomSkillIncreaseRegSet.h"

namespace CustomSkills
{
	CustomSkillIncreaseRegSet* CustomSkillIncreaseRegSet::Get()
	{
		static CustomSkillIncreaseRegSet instance{};
		return &instance;
	}
}
