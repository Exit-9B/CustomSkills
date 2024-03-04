#pragma once

namespace CustomSkills
{
	class Constellation final
	{
	public:
		static void WriteHooks();

	private:
		// Initialize constellation image controllers
		static void LoadSkydomePatch1();

		// Update constellation when finishing rotation
		static void EnterConstellationPatch1();
		static void EnterConstellationPatch2();

		// Update constellation when starting rotation
		static void ExitConstellationPatch1();
		static void ExitConstellationPatch2();

		// Change active constellation with Kinect input
		static void KinectPatch();

		// Update constellation alpha
		static void UpdateConstellationPatch();
	};
}
