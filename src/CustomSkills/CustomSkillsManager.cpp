#include "CustomSkillsManager.h"

#include "Game.h"
#include "RE/Offset.h"
#include "Settings.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void CustomSkillsManager::Initialize()
	{
		auto& trampoline = SKSE::GetTrampoline();
		IsSingleSkillMode = reinterpret_cast<std::uintptr_t>(trampoline.allocate(1));
		IsUsingBeastNif = reinterpret_cast<std::uintptr_t>(trampoline.allocate(1));
		ShouldHideLevel = reinterpret_cast<std::uintptr_t>(trampoline.allocate(1));
		UpdateVars();
	}

	void CustomSkillsManager::LoadSkills()
	{
		_skillIds = Settings::ReadSkills();

		_skills.clear();

		for (auto& [key, skill] : _skillIds) {
			_skills.push_back(skill);
		}
	}

	void CustomSkillsManager::ShowLevelup(std::string_view a_name, std::int32_t a_level)
	{
		static auto sSkillIncreased = RE::GameSettingCollection::GetSingleton()->GetSetting(
			"sSkillIncreased");

		if (!sSkillIncreased) {
			return;
		}

		const char* text = sSkillIncreased->GetString();
		if (!text || text[0] == '\0') {
			return;
		}

		char buf[200];
		std::snprintf(buf, 200, text, a_name.data(), a_level);

		Game::ShowHUDMessage(RE::HUDData::Type::kSkillIncrease, buf, nullptr, nullptr);
	}

	void CustomSkillsManager::CloseStatsMenu()
	{
		if (auto uiMessageQueue = RE::UIMessageQueue::GetSingleton()) {
			SetMenuState(MenuState::WaitingToClose);
			uiMessageQueue->AddMessage(MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}

	void CustomSkillsManager::OpenStatsMenu(std::shared_ptr<Skill> a_skill)
	{
		_menuSkill = a_skill;

		if (!_menuSkill || !_menuSkill->SkillTree) {
			return;
		}

		auto av = Game::GetActorValueInfo(MENU_AV);
		if (!av) {
			return;
		}

		auto pt = av->perkTree;
		if (!pt) {
			return;
		}

		if (!_originalSkillTree) {
			_originalSkillTree = pt;
			_originalSkillTreeWidth = av->perkTreeWidth;
		}

		auto nt = _menuSkill->SkillTree;
		if (nt != pt) {
			av->perkTree = nt;
			// Vanilla Enchanting value for backwards compatibility
			av->perkTreeWidth = 3;
		}

		SetMenuState(MenuState::WaitingToOpen);

		Game::FadeOutGame(true, true, 1.0f, true, 0.0f);
		Game::OpenStatsMenu(false);
	}

	bool CustomSkillsManager::IsMenuControlsEnabled()
	{
		return RE::ControlMap::GetSingleton()->IsMainFourControlsEnabled();
	}

	bool CustomSkillsManager::IsStatsMenuOpen()
	{
		return RE::UI::GetSingleton()->IsMenuOpen(MENU_NAME);
	}

	std::uint32_t CustomSkillsManager::GetCurrentSkillCount()
	{
		if (IsOurMenuMode()) {
			return 1;
		}
		else if (IsBeastMode()) {
			return 1;
		}
		else {
			return 18;
		}
	}

	std::uint32_t CustomSkillsManager::GetCurrentPerkPoints()
	{
		if (IsOurMenuMode()) {
			if (_menuSkill && _menuSkill->PerkPoints) {

				auto v = _menuSkill->PerkPoints->value;

				if (v <= 0) {
					return 0;
				}

				if (v > 255) {
					return 255;
				}

				return static_cast<std::uint32_t>(v);
			}
		}

		if (const auto player = RE::PlayerCharacter::GetSingleton()) {
			return static_cast<std::uint32_t>(player->perkCount);
		}

		return 0;
	}

	void CustomSkillsManager::SetCurrentPerkPoints(std::uint8_t a_value)
	{
		if (IsOurMenuMode()) {
			if (_menuSkill && _menuSkill->PerkPoints) {
				_menuSkill->PerkPoints->value = a_value;
				return;
			}
		}

		if (auto player = RE::PlayerCharacter::GetSingleton()) {
			player->perkCount = a_value;
		}
	}

	void CustomSkillsManager::SetMenuState(MenuState a_state)
	{
		_menuState = a_state;
		UpdateVars();
	}

	void CustomSkillsManager::SetBeastMode(bool a_beastMode)
	{
		REL::Relocation<bool*> isBeastMode{ REL::ID(RE::Offset::IsBeastMode) };
		*isBeastMode.get() = a_beastMode;
		UpdateVars();
	}

	bool CustomSkillsManager::IsOurMenuMode()
	{
		return _menuState != MenuState::None && _menuSkill != nullptr;
	}

	bool CustomSkillsManager::IsBeastMode()
	{
		REL::Relocation<bool*> isBeastMode{ RE::Offset::IsBeastMode };
		return *isBeastMode.get();
	}

	float CustomSkillsManager::GetSkillLevel(RE::ActorValue a_skill)
	{
		if (IsOurMenuMode()) {
			return _menuSkill->GetLevel();
		}

		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetActorValue(a_skill);
	}

	float CustomSkillsManager::GetSkillProgressPercent(RE::ActorValue a_skill)
	{
		if (IsOurMenuMode()) {
			return _menuSkill->Ratio ? _menuSkill->Ratio->value * 100.0f : 0.0f;
		}

		// WerewolfPerks / VampirePerks store skill progress
		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetActorValue(a_skill);
	}

	float CustomSkillsManager::GetBaseSkillLevel(RE::ActorValue a_skill)
	{
		if (IsOurMenuMode()) {
			return _menuSkill->GetLevel();
		}

		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetBaseActorValue(a_skill);
	}

	std::shared_ptr<Skill> CustomSkillsManager::FindSkill(const std::string& a_key)
	{
		if (auto i = _skillIds.find(a_key); i != _skillIds.end()) {
			return i->second;
		}

		return nullptr;
	}

	std::shared_ptr<Skill> CustomSkillsManager::FindSkillFromGlobalLevel(RE::TESGlobal* a_global)
	{
		for (auto& sk : _skills) {
			if (sk->Level && sk->Level->type == RE::TESGlobal::Type::kShort) {
				if (sk->Level == a_global) {
					return sk;
				}
			}
		}

		return nullptr;
	}

	void CustomSkillsManager::UpdateSkills()
	{
		if (RE::Main::GetSingleton()->freezeTime || RE::UI::GetSingleton()->GameIsPaused()) {
			return;
		}

		// Don't update anything if we are already in menu or the menu controls are disabled. This
		// is usually when some special game event is taking place.
		if (IsStatsMenuOpen() || !IsMenuControlsEnabled()) {
			return;
		}

		bool reload = false;
		for (auto& sk : _skills) {
			if (sk->OpenMenu) {
				auto amt = static_cast<std::int16_t>(sk->OpenMenu->value);
				if (amt > 0) {
					sk->OpenMenu->value = 0;

					OpenStatsMenu(sk);
					return;
				}
			}

			if (sk->ShowLevelup) {
				auto amt = static_cast<std::int16_t>(sk->ShowLevelup->value);
				if (amt > 0) {
					sk->ShowLevelup->value = 0;

					ShowLevelup(sk->Name, amt);
					return;
				}
			}

			if (sk->DebugReload) {
				if (sk->DebugReload->value > 0) {
					sk->DebugReload->value = 0;

					reload = true;
					break;
				}
			}
		}

		if (reload && !IsStatsMenuOpen()) {
			LoadSkills();
		}
	}

	void CustomSkillsManager::UpdateMenu()
	{
		bool closed = false;
		switch (_menuState) {
		case MenuState::None:
			break;

		case MenuState::WaitingToOpen:
			if (IsStatsMenuOpen()) {
				SetMenuState(MenuState::Open);
			}
			break;

		case MenuState::Open:
			if (!IsStatsMenuOpen()) {
				closed = true;
				SetMenuState(MenuState::None);
			}
			break;

		case MenuState::WaitingToClose:
			if (!IsStatsMenuOpen()) {
				closed = true;
				SetMenuState(MenuState::None);
			}
			break;
		}

		if (closed) {
			if (_originalSkillTree) {
				if (auto av = Game::GetActorValueInfo(MENU_AV)) {
					av->perkTree = _originalSkillTree;
					av->perkTreeWidth = _originalSkillTreeWidth;
				}
			}
		}
	}

	void CustomSkillsManager::UpdateVars()
	{
		bool modeOn = _menuState != MenuState::None && _menuSkill != nullptr;
		*IsSingleSkillMode.get() = modeOn || IsBeastMode();
		*IsUsingBeastNif.get() = modeOn ? !_menuSkill->NormalNif : IsBeastMode();
		*ShouldHideLevel.get() = (modeOn && _menuSkill->Level == nullptr) || IsBeastMode();
	}
}
