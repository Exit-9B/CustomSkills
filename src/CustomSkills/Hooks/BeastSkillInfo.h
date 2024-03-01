#pragma once

namespace CustomSkills
{
	class BeastSkillInfo final
	{
	public:
		static void WriteHooks();

	private:
		// Display as Beast Skill (no level)
		static void BeastSkillPatch();

		// Skill name in tree view
		static void SkillNamePatch();

		// Re-show Beast Skill info when zooming out
		static void ZoomOutPatch();

		// No skill level in perk description
		static void PerkViewPatch();
	};
}
