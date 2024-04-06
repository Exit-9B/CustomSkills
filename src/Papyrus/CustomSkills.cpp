#include "CustomSkills.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Game.h"

#define REGISTER(vm, func) vm->RegisterFunction(#func##sv, "CustomSkills"sv, func)

using namespace CustomSkills;

namespace Papyrus::CustomSkills
{
	std::int32_t GetAPIVersion(RE::StaticFunctionTag*)
	{
		return 2;
	}

	void OpenCustomSkillMenu(RE::StaticFunctionTag*, std::string asSkillId)
	{
		if (const auto group = CustomSkillsManager::FindSkillMenu(asSkillId)) {
			CustomSkillsManager::OpenStatsMenu(group);
		}
		else if (const auto [origin, index] = CustomSkillsManager::FindSkillOrigin(asSkillId);
				 origin) {
			origin->LastSelectedTree = static_cast<std::uint32_t>(index);
			CustomSkillsManager::OpenStatsMenu(origin);
		}
	}

	void ShowTrainingMenu(
		RE::StaticFunctionTag*,
		std::string asSkillId,
		std::int32_t aiMaxLevel,
		RE::Actor* akTrainer)
	{
		if (const auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			CustomSkillsManager::ShowTrainingMenu(skill, aiMaxLevel, akTrainer);
		}
	}

	void AdvanceSkill(RE::StaticFunctionTag*, std::string asSkillId, float afMagnitude) {
		if (const auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			skill->Advance(afMagnitude);
		}
	}

	void IncrementSkill(RE::StaticFunctionTag*, std::string asSkillId) {
		if (const auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			skill->Increment(1);
		}
	}

	void IncrementSkillBy(RE::StaticFunctionTag*, std::string asSkillId, std::int32_t aiCount) {
		if (const auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			skill->Increment(aiCount);
		}
	}

	void ShowSkillIncreaseMessage(
		RE::StaticFunctionTag*,
		std::string asSkillId,
		std::int32_t aiSkillLevel)
	{
		if (const auto skill = CustomSkillsManager::FindSkill(asSkillId)) {
			Game::ShowSkillIncreasedMessage(skill->GetName(), aiSkillLevel);
		}
	}

	void DebugReload(RE::StaticFunctionTag*)
	{
		CustomSkillsManager::LoadSkills();
	}

	bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
	{
		REGISTER(a_vm, GetAPIVersion);
		REGISTER(a_vm, OpenCustomSkillMenu);
		REGISTER(a_vm, ShowTrainingMenu);
		REGISTER(a_vm, AdvanceSkill);
		REGISTER(a_vm, IncrementSkill);
		REGISTER(a_vm, IncrementSkillBy);
		REGISTER(a_vm, ShowSkillIncreaseMessage);
		REGISTER(a_vm, DebugReload);

		return true;
	}
}
