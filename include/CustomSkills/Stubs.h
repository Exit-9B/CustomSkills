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

	namespace detail
	{
		struct CustomSkillsInterface
		{
			std::uint32_t interfaceVersion;
			void (*AdvanceSkill)(const char* a_skillId, float a_magnitude);
			void (*IncrementSkill)(const char* a_skillId, std::uint32_t a_count);
		};
		static_assert(sizeof(CustomSkillsInterface) == 0x18);
	}
}
