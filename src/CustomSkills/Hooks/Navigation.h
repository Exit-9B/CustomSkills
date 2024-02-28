#pragma once

namespace CustomSkills
{
	class Navigation final
	{
	public:
		static void WriteHooks();

	private:
		// Prevent tree switching
		static void LockRotationPatch();

		// Normalize rotation speed
		static void RotationSpeedPatch();
	};
}
