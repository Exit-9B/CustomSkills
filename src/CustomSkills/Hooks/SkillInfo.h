#pragma once

namespace CustomSkills
{
	class SkillInfo final
	{
	public:
		static void WriteHooks();

	private:
		// Skill name in tree view
		static void SkillNamePatch();

		// Skill name color
		static void SkillColorPatch();

		// Skill description
		static void SkillDescriptionPatch();
	};
}
