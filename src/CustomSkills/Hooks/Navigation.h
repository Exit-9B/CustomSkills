#pragma once

namespace CustomSkills
{
	class Navigation final
	{
	public:
		static void WriteHooks();

	private:
		// Don't allow left right tree switching
		static void NavigatePatch();

		// Don't remember last selected tree
		static void CloseMenuPatch();

		// No horizontal velocity
		static void RotatePatch();
	};
}
