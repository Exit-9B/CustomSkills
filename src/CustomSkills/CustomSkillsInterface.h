#pragma once

#include "CustomSkills/Stubs.h"

namespace CustomSkills::Impl
{
	namespace CustomSkillsInterface
	{
		constexpr std::uint32_t InterfaceVersion = 1;

		void Dispatch();

		detail::CustomSkillsInterface* Get();

		void AdvanceSkill(const char* a_skillId, float a_magnitude);

		void IncrementSkill(const char* a_skillId, std::uint32_t a_count);

		void* GetEventDispatcher(std::uint32_t a_dispatcherID);
	}
}
