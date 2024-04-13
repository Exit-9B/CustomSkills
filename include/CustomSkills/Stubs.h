/**
 * @file Stubs.h
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <cstdint>

namespace CustomSkills
{
	enum MESSAGE_TYPE : std::uint32_t
	{
		kCustomSkillsInterface,
	};

	enum class Dispatcher : std::uint32_t
	{
		kSkillIncreaseEvent,
	};

	namespace detail
	{
		struct CustomSkillsInterface
		{
			std::uint32_t interfaceVersion;
			void (*ShowStatsMenu)(const char* a_skillId);
			void (*AdvanceSkill)(const char* a_skillId, float a_magnitude);
			void (*IncrementSkill)(const char* a_skillId, std::uint32_t a_count);
			void* (*GetEventDispatcher)(std::uint32_t a_dispatcherID);
		};
		static_assert(sizeof(CustomSkillsInterface) == 0x28);
	}
}
