#pragma once

namespace Papyrus
{
	namespace CustomSkills
	{
		std::int32_t GetAPIVersion(RE::StaticFunctionTag*);

		void OpenCustomSkillMenu(RE::StaticFunctionTag*, std::string asSkillId);

		void ShowCustomTrainingMenu(
			RE::StaticFunctionTag*,
			std::string asSkillId,
			std::int32_t aiMaxLevel,
			RE::Actor* akTrainer);

		void ShowSkillIncreaseMessage(
			RE::StaticFunctionTag*,
			std::string asSkillId,
			std::int32_t aiSkillLevel);

		bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm);
	}
}
