#pragma once

namespace CustomSkills
{
	namespace Game
	{
		void FadeOutGame(
			bool a_fadingOut,
			bool a_blackFade,
			float a_fadeDuration,
			bool a_arg4,
			float a_secsBeforeFade);

		RE::ActorValueInfo* GetActorValueInfo(RE::ActorValue a_actorValue);

		const char* GetActorValueName(RE::ActorValue a_actorValue);

		const char* GetActorValueColor(RE::ActorValue a_actorValue);

		bool IsGamePaused();

		void OpenStatsMenu(bool a_isBeastMode);

		void ShowTrainingMenu(RE::Actor* a_trainer);

		void ShowHUDMessage(
			RE::HUDData::Type a_messageType,
			const char* a_message,
			RE::TESQuest* a_owningQuest = nullptr,
			RE::BGSQuestObjective* a_questObjective = nullptr);

		void ShowSkillIncreasedMessage(std::string_view a_name, std::int32_t a_level);
	}
}
