#pragma once

namespace CustomSkills
{
	class Crafting final
	{
	public:
		static void WriteHooks();

	private:
		// Skill info in constructible object bottom bar
		static void ConstructibleObjectBottomBarPatch();

		// Custom skill XP for constructible object workbenches
		static void ConstructibleObjectCreationPatch();
	};
}
