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

		// Re-show Beast Skill info when zooming out
		static void ZoomOutPatch();
	};
}
