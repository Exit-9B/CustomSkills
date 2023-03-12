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

		// Skill name in perk description
		static void PerkSkillNamePatch();

		// Skill description
		static void SkillDescriptionPatch();
	};
}
