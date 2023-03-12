#pragma once

namespace CustomSkills
{
	class MenuSetup final
	{
	public:
		static void WriteHooks();

	private:
		// Open menu at correct index
		static void MenuPropertiesPatch();

		// Skill dome nif
		static void SkillDomeArtPatch();

		// Fix camera when using beast skill nif
		static void CameraPatch();

		// Create array for only one skill
		static void SkillArrayPatch();

		// Update data for only one skill
		static void UpdateSkillPatch();

		// Create perk nodes for only one skill
		static void CreateStarsPatch();
	};
}
