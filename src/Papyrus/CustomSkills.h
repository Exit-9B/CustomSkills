#pragma once

namespace Papyrus
{
	namespace CustomSkills
	{
		std::int32_t GetAPIVersion(RE::StaticFunctionTag*);

		void OpenCustomSkillMenu(RE::StaticFunctionTag*, std::string asSkillId);

		void ShowTrainingMenu(
			RE::StaticFunctionTag*,
			std::string asSkillId,
			std::int32_t aiMaxLevel,
			RE::Actor* akTrainer);

		void AdvanceSkill(RE::StaticFunctionTag*, std::string asSkillId, float afMagnitude);

		void IncrementSkill(RE::StaticFunctionTag*, std::string asSkillId);

		void IncrementSkillBy(RE::StaticFunctionTag*, std::string asSkillId, std::int32_t aiCount);

		void ShowSkillIncreaseMessage(
			RE::StaticFunctionTag*,
			std::string asSkillId,
			std::int32_t aiSkillLevel);

		void DebugReload(RE::StaticFunctionTag*);

		bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm);
	}
}
