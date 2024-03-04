#include "Game.h"

#include "RE/Offset.h"

namespace CustomSkills
{
	void Game::FadeOutGame(
		bool a_fadingOut,
		bool a_blackFade,
		float a_fadeDuration,
		bool a_arg4,
		float a_secsBeforeFade)
	{
		using func_t = decltype(&FadeOutGame);
		REL::Relocation<func_t> func{ RE::Offset::FadeOutGame };
		return func(a_fadingOut, a_blackFade, a_fadeDuration, a_arg4, a_secsBeforeFade);
	}

	RE::ActorValueInfo* Game::GetActorValueInfo(RE::ActorValue a_actorValue)
	{
		using func_t = decltype(&GetActorValueInfo);
		REL::Relocation<func_t> func{ RE::Offset::GetActorValueInfo };
		return func(a_actorValue);
	}

	const char* Game::GetActorValueName(RE::ActorValue a_actorValue)
	{
		using func_t = decltype(&GetActorValueName);
		REL::Relocation<func_t> func{ RE::Offset::GetActorValueName };
		return func(a_actorValue);
	}

	const char* Game::GetActorValueColor(RE::ActorValue a_actorValue)
	{
		using func_t = decltype(&GetActorValueColor);
		REL::Relocation<func_t> func{ RE::Offset::GetActorValueColor };
		return func(a_actorValue);
	}

	bool Game::IsGamePaused()
	{
		if (const auto main = RE::Main::GetSingleton()) {
			if (main->freezeTime) {
				return true;
			}
		}

		REL::Relocation<bool*> isInMenuMode{ RE::Offset::IsInMenuMode };
		if (*isInMenuMode.get()) {
			return true;
		}

		if (const auto ui = RE::UI::GetSingleton()) {
			if (ui->GameIsPaused()) {
				return true;
			}
		}

		return false;
	}

	void Game::OpenStatsMenu(bool a_isBeastMode)
	{
		using func_t = decltype(&OpenStatsMenu);
		REL::Relocation<func_t> func{ RE::Offset::OpenStatsMenu };
		return func(a_isBeastMode);
	}

	void Game::ShowHUDMessage(
		RE::HUDData::Type a_messageType,
		const char* a_message,
		RE::TESQuest* a_owningQuest,
		RE::BGSQuestObjective* a_questObjective)
	{
		using func_t = decltype(&ShowHUDMessage);
		REL::Relocation<func_t> func{ RE::Offset::ShowHUDMessage };
		return func(a_messageType, a_message, a_owningQuest, a_questObjective);
	}
}
