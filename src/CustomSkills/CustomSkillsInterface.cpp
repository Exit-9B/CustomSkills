#include "CustomSkillsInterface.h"

#include "CustomSkillsManager.h"
#include "EventSources.h"

namespace CustomSkills::Impl
{
	void CustomSkillsInterface::Dispatch()
	{
		SKSE::GetMessagingInterface()->Dispatch(
			kCustomSkillsInterface,
			Get(),
			sizeof(detail::CustomSkillsInterface),
			nullptr);
	}

	detail::CustomSkillsInterface* CustomSkillsInterface::Get()
	{
		static detail::CustomSkillsInterface intfc{
			.interfaceVersion = InterfaceVersion,
			.ShowStatsMenu = &ShowStatsMenu,
			.AdvanceSkill = &AdvanceSkill,
			.IncrementSkill = &IncrementSkill,
			.GetEventDispatcher = &GetEventDispatcher,
		};
		return std::addressof(intfc);
	}

	void CustomSkillsInterface::ShowStatsMenu(const char* a_skillId)
	{
		if (const auto group = CustomSkillsManager::FindSkillMenu(a_skillId)) {
			CustomSkillsManager::OpenStatsMenu(group);
		}
		else if (const auto [origin, index] = CustomSkillsManager::FindSkillOrigin(a_skillId);
				 origin) {
			origin->LastSelectedTree = static_cast<std::uint32_t>(index);
			CustomSkillsManager::OpenStatsMenu(origin);
		}
	}

	void CustomSkillsInterface::AdvanceSkill(const char* a_skillId, float a_magnitude)
	{
		if (const auto skill = CustomSkillsManager::FindSkill(a_skillId)) {
			skill->Advance(a_magnitude);
		}
	}

	void CustomSkillsInterface::IncrementSkill(const char* a_skillId, std::uint32_t a_count)
	{
		if (const auto skill = CustomSkillsManager::FindSkill(a_skillId)) {
			skill->Increment(a_count);
		}
	}

	void* CustomSkillsInterface::GetEventDispatcher(std::uint32_t a_dispatcherID)
	{
		switch (static_cast<Dispatcher>(a_dispatcherID)) {
		case Dispatcher::kSkillIncreaseEvent:
			return SkillIncreaseEventSource::Get();
		}
		return nullptr;
	}
}
