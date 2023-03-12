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

		bool IsGamePaused();

		void OpenStatsMenu(bool a_isBeastMode);

		void ShowHUDMessage(
			RE::HUDData::Type a_messageType,
			const char* a_message,
			RE::TESQuest* a_owningQuest = nullptr,
			RE::BGSQuestObjective* a_questObjective = nullptr);
	}
}
