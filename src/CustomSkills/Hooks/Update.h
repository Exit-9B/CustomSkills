#pragma once

#include "CustomSkills/Skill.h"

namespace CustomSkills
{
	class Update final
	{
	public:
		static void WriteHooks();

	private:
		// Update each frame
		static void FrameHook();

		// Update variables when opening menu
		static void SetBeastModePatch();

		// Update variables when closing menu
		static void ExitModePatch();
	};
}
