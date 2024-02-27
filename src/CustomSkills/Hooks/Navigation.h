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

		// Prevent fading of constellation image
		static void LockShaderPatch();

		// Normalize rotation speed
		static void RotationSpeedPatch();
	};
}
