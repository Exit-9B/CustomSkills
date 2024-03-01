#pragma once

namespace CustomSkills
{
	class Legendary final
	{
	public:
		static void WriteHooks();

	private:
		// Reset skill level in custom global variable
		static void ResetSkillLevelPatch();

		// Skip setting data in player skills struct
		static void PlayerSkillsPatch();

		// Refund perks to custom global variable
		static void RefundPerksPatch();
	};
}
