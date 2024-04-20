#pragma once

namespace CustomSkills
{
	class SkillProgress final
	{
	public:
		static void WriteHooks();

	private:
		// Show skill level and XP
		static void SkillProgressPatch();

		// Show available perk points
		static void CurrentPerkPointsPatch();

		// Spend perk points
		static void SelectPerkPatch();

		// Hide skill level in perk description
		static void HideLevelPatch();

		// Set requirements text when using GetGlobalValue
		static void RequirementsTextPatch();

		static void ModifyPerkCount(RE::StatsMenu* a_statsMenu, std::int32_t a_countDelta);

		static void GetSkillProgress(
			RE::PlayerCharacter::PlayerSkills* a_playerSkills,
			RE::ActorValue a_skill,
			float* a_level,
			float* a_xp,
			float* a_levelThreshold,
			std::uint32_t* a_legendary);

		static void GetRequirementsText(
			RE::BGSPerk* a_perk,
			char* a_buf,
			std::int32_t a_bufLen,
			const char* a_prefix,
			const char* a_suffix);

		inline static REL::Relocation<decltype(&ModifyPerkCount)> _ModifyPerkCount;
	};
}
