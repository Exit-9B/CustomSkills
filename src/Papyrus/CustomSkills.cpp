#include "CustomSkills.h"

#include "CustomSkills/CustomSkillsManager.h"

#define REGISTER(vm, func)  vm->RegisterFunction(#func##sv, "CustomSkills"sv, func)

using namespace CustomSkills;

namespace Papyrus::CustomSkills
{
	std::int32_t GetAPIVersion(RE::StaticFunctionTag*)
	{
		return 1;
	}

	void OpenCustomSkillMenu(RE::StaticFunctionTag*, RE::BSFixedString asSkillId)
	{
		if (auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			CustomSkillsManager::OpenStatsMenu(skill);
		}
	}

	void ShowSkillIncreaseMessage(
		RE::StaticFunctionTag*,
		RE::BSFixedString asSkillId,
		std::int32_t aiSkillLevel)
	{
		if (auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			CustomSkillsManager::ShowLevelup(skill->Name, aiSkillLevel);
		}
	}

	bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
	{
		REGISTER(a_vm, GetAPIVersion);
		REGISTER(a_vm, OpenCustomSkillMenu);
		REGISTER(a_vm, ShowSkillIncreaseMessage);

		return true;
	}
}
