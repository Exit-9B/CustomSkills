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

		// Get custom skill tree
		static void SkillTreePatch();

		// Save last selected tree
		static void CloseMenuPatch();
	};
}
