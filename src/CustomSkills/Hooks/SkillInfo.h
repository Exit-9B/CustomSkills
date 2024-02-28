#pragma once

namespace CustomSkills
{
	class SkillInfo final
	{
	public:
		static void WriteHooks();

	private:
		// Skill stats in ring
		static void SkillStatsPatch();

		// Skill description
		static void SkillDescriptionPatch();
	};
}
