/**
 * @file Interfaces.inl
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Interfaces.h"

namespace CustomSkills
{
	inline std::uint32_t CustomSkillsInterface::Version() const
	{
		return GetProxy()->interfaceVersion;
	}

	inline void CustomSkillsInterface::AdvanceSkill(const char* a_skillId, float a_magnitude)
	{
		GetProxy()->AdvanceSkill(a_skillId, a_magnitude);
	}

	inline void CustomSkillsInterface::IncrementSkill(const char* a_skillId, std::uint32_t a_count)
	{
		GetProxy()->IncrementSkill(a_skillId, a_count);
	}

	inline const detail::CustomSkillsInterface* CustomSkillsInterface::GetProxy() const
	{
		return reinterpret_cast<const detail::CustomSkillsInterface*>(this);
	}

	inline void QueryCustomSkillsInterface(
		const SKSE::MessagingInterface::Message* a_msg,
		CustomSkillsInterface*& a_intfc)
	{
		if (!a_msg || ::strcmp(a_msg->sender, "CustomSkills") != 0 ||
			a_msg->type != kCustomSkillsInterface) {
			return;
		}

		auto result = static_cast<CustomSkillsInterface*>(a_msg->data);

		if (result && result->Version() > CustomSkillsInterface::kVersion) {
			SKSE::log::warn("interface definition is out of date"sv);
		}

		a_intfc = result;
	}
}
