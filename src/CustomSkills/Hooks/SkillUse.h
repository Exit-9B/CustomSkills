#pragma once

namespace CustomSkills
{
	class SkillUse final
	{
	public:
		static void WriteHooks();

	private:
		// Handle weapon XP
		static void UseSkillPatch();

		// Skill info in constructible object bottom bar
		static void ConstructibleObjectBottomBarPatch();

		// Custom skill XP for constructible object workbenches
		static void ConstructibleObjectCreationPatch();

		static void UseSkill(
			RE::PlayerCharacter* a_player,
			RE::ActorValue a_skill,
			float a_amount,
			RE::TESForm* a_advanceObject,
			std::uint32_t a_advanceAction);

		inline static REL::Relocation<decltype(&UseSkill)> _UseSkill;
	};
}
