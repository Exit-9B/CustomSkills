#pragma once

namespace CustomSkills
{
	class SkillProgress final
	{
	public:
		static void WriteHooks();

	private:
		// Show available perk points
		static void CurrentPerkPointsPatch();

		// Spend or add perk points
		static void ModifyPerkPointsPatch();

		// Show skill level and XP
		static void SkillProgressPatch();

		// Skill level in tree view
		static void SkillLevelPatch();

		// Unknown
		static void SkillLevelPatch2();

		// Skill level in perk description
		static void PerkViewSkillLevelPatch();

		// Set requirements text when using GetGlobalValue
		static void RequirementsTextPatch();

		static void ModifyPerkCount(std::int32_t a_countDelta);

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
	};
}
