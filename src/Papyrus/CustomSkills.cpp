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

	void OpenCustomSkillMenu(RE::StaticFunctionTag*, std::string asSkillId)
	{
		if (const auto group = CustomSkillsManager::FindSkillMenu(asSkillId)) {
			CustomSkillsManager::OpenStatsMenu(group);
		}
	}

	void ShowSkillIncreaseMessage(
		RE::StaticFunctionTag*,
		std::string asSkillId,
		std::int32_t aiSkillLevel)
	{
		if (auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			CustomSkillsManager::ShowLevelup(skill->GetName(), aiSkillLevel);
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
