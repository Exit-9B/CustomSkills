/**
 * @file Events.h
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Stubs.h"

namespace CustomSkills
{
	struct SkillIncreaseEvent
	{
		static constexpr Dispatcher ID = Dispatcher::kSkillIncreaseEvent;

		const char* skillID;
	};
	static_assert(sizeof(SkillIncreaseEvent) == 0x8);
}
