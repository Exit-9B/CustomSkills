#pragma once

namespace CustomSkills
{
	class Navigation final
	{
	public:
		static void WriteHooks();

	private:
		// Compute index of current angle
		static void SelectedTreePatch();

		// Prevent tree switching
		static void LockRotationPatch();

		// Normalize rotation speed
		static void RotationSpeedPatch();
	};
}
