/**
 * @file Interfaces.h
 *
 * Copyright (c) Parapets
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "Events.h"
#include "Stubs.h"

#include "SKSE/Interfaces.h"

namespace RE
{
	template <class Event>
	class BSTEventSource;
}

namespace CustomSkills
{
	class CustomSkillsInterface
	{
	public:
		enum
		{
			kVersion = 1,
		};

		/**
		 * Get the version of the interface.
		 */
		[[nodiscard]] std::uint32_t Version() const;

		/**
		 * Open the custom skill menu for the given skill or group (config file).
		 *
		 * @param[in] a_skillId The ID of the skill or group to open the menu for.
		 */
		void ShowStatsMenu(const char* a_skillId);

		/**
		 * Advance the given skill by the provided amount of skill usage.
		 *
		 * @param[in] a_skillId   The ID of the skill to advance.
		 * @param[in] a_magnitude The amount of XP gained.
		 */
		void AdvanceSkill(const char* a_skillId, float a_magnitude);

		/**
		 * Increment the given skill by the given number of points.
		 *
		 * @param[in] a_skillId The ID of the skill to increment.
		 * @param[in] a_count   The number of points to increment the skill rank.
		 */
		void IncrementSkill(const char* a_skillId, std::uint32_t a_count = 1);

		/**
		 * Get an event dispatcher.
		 *
		 * @tparam T The event type.
		 */
		template <class T>
		[[nodiscard]] RE::BSTEventSource<T>* GetEventDispatcher();

	protected:
		[[nodiscard]] const detail::CustomSkillsInterface* GetProxy() const;
	};

	/**
	 * Try to get the CustomSkillsInterface from a message.
	 *
	 * @param[in]  a_msg   The message sent by Custom Skills Framework.
	 * @param[out] a_intfc The variable to store the interface, if present.
	 */
	void QueryCustomSkillsInterface(
		const SKSE::MessagingInterface::Message* a_msg,
		CustomSkillsInterface*& a_intfc);
}

#include "Interfaces.inl"
