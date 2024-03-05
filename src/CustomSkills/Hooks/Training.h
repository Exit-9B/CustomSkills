#pragma once

namespace CustomSkills
{
	class Training final
	{
	public:
		static void WriteHooks();

	private:
		// Set to unused actor value index internally
		static void MenuSkillPatch();

		// Custom skill name in menu
		static void SkillNamePatch();

		// Maximum training level
		static void MaxLevelPatch();

		// Increment custom skill
		static void IncrementSkillPatch();
	};
}
