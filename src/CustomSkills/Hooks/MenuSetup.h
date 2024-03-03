#pragma once

namespace CustomSkills
{
	class MenuSetup final
	{
	public:
		static void WriteHooks();

	private:
		// Setup custom menu properties
		static void MenuConstructorPatch();

		// Skill dome nif
		static void SkillDomeArtPatch();

		// Fix camera when using beast skill nif
		static void CameraPatch();

		// Get custom skill tree
		static void SkillTreePatch();

		// Create stars for more than 18 skills
		static void CreateStarsPatch();

		// Save last selected tree
		static void CloseMenuPatch();
	};
}
