#pragma once

namespace CustomSkills
{
	class Navigation final
	{
	public:
		static void WriteHooks();

	private:
		// Prevent tree switching
		static void RotatePatch();

		// Prevent fading of constellation image
		static void EffectShaderPatch();
	};
}
