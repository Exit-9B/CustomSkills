#include "CustomSkillsInterface.h"

#include "CustomSkillsManager.h"
#include "SkillIncreaseEventSource.h"

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
			.AdvanceSkill = &AdvanceSkill,
			.IncrementSkill = &IncrementSkill,
			.GetEventDispatcher = &GetEventDispatcher,
		};
		return std::addressof(intfc);
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
			return SkillIncreaseEventSource::Instance();
		}
		return nullptr;
	}
}
