#pragma once

namespace CustomSkills
{
	class Legendary final
	{
	public:
		static void WriteHooks();

	private:
		// Show legendary hint at level 100
		static void LegendaryHintPatch();

		// Legendary reset when pressing Jump/YButton
		static void ProcessButtonPatch();

		// Legendary reset when pressing Space
		static void ProcessMessagePatch();

		// Check skill level again when confirming
		static void LegendarySkillConfirmPatch();

		// Reset skill level in custom global variable
		static void ResetSkillLevelPatch();

		// Skip setting data in player skills struct
		static void PlayerSkillsPatch();

		// Check whether legendary should be available
		static void LegendaryAvailablePatch();

		// Refund perks to custom global variable
		static void RefundPerksPatch();

		static void LegendaryReset(
			RE::ActorValueOwner* a_avOwner,
			RE::ActorValue a_skill,
			float a_resetValue);
	};
}
