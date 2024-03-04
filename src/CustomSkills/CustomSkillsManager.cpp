#include "CustomSkillsManager.h"

#include "Game.h"
#include "RE/Offset.h"
#include "Settings.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void CustomSkillsManager::Initialize()
	{
		// TRAMPOLINE: 6
		auto& trampoline = SKSE::GetTrampoline();
		IsSingleSkillMode = reinterpret_cast<std::uintptr_t>(trampoline.allocate(1));
		UseBeastSkillInfo = reinterpret_cast<std::uintptr_t>(trampoline.allocate(1));
		CameraRightPoint = reinterpret_cast<std::uintptr_t>(trampoline.allocate(4));
		UpdateVars();
	}

	void CustomSkillsManager::LoadSkills()
	{
		_groupIds = Settings::ReadSkills();
		_skillIds.clear();
		_requirementSkills.clear();

		for (const auto& [key, group] : _groupIds) {
			for (std::size_t i = 0; i < group->Skills.size(); ++i) {
				const auto& skill = group->Skills[i];
				if (!skill)
					continue;

				const auto& id = !skill->ID.empty() ? skill->ID : key;
				_skillIds.emplace(id, std::make_pair(group, i));

				if (skill->Level && skill->Level->type == RE::TESGlobal::Type::kShort) {
					_requirementSkills.emplace(skill->Level, skill);
				}
			}
		}
	}

	void CustomSkillsManager::CloseStatsMenu()
	{
		if (const auto uiMessageQueue = RE::UIMessageQueue::GetSingleton()) {
			SetMenuState(MenuState::WaitingToClose);
			uiMessageQueue
				->AddMessage(RE::StatsMenu::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
		}
	}

	void CustomSkillsManager::OpenStatsMenu(std::shared_ptr<SkillGroup> a_group)
	{
		if (!a_group || a_group->Skills.empty()) {
			return;
		}

		_menuSkills = a_group;
		SetMenuState(MenuState::WaitingToOpen);

		Game::FadeOutGame(true, true, 1.0f, true, 0.0f);
		Game::OpenStatsMenu(false);
	}

	void CustomSkillsManager::ShowTrainingMenu(
		std::shared_ptr<Skill> a_skill,
		std::uint32_t a_maxLevel,
		RE::Actor* a_trainer)
	{
		_trainingSkill = a_skill;
		_trainingMax = a_maxLevel;
		SetTrainingState(MenuState::WaitingToOpen);

		Game::ShowTrainingMenu(a_trainer);
	}

	void CustomSkillsManager::NotifyOpeningSkills()
	{
		if (!IsBeastMode() && _menuState != MenuState::WaitingToOpen) {
			const auto it = _groupIds.find("SKILLS"s);
			if (it != _groupIds.end() && it->second) {
				_menuSkills = it->second;
				SetMenuState(MenuState::WaitingToOpen);
			}
		}

		_cImageControllers.clear();
		_cImageControllers.resize((std::max)(GetCurrentSkillCount(), 2U));
	}

	bool CustomSkillsManager::IsMenuControlsEnabled()
	{
		static REL::Relocation<bool()> func{ REL::ID(55484) };
		return func();
	}

	bool CustomSkillsManager::IsStatsMenuOpen()
	{
		return RE::UI::GetSingleton()->IsMenuOpen(RE::StatsMenu::MENU_NAME);
	}

	std::uint32_t CustomSkillsManager::GetCurrentSkillCount()
	{
		if (IsOurMenuMode()) {
			return static_cast<std::uint32_t>(_menuSkills->Skills.size());
		}
		else if (IsBeastMode()) {
			return 2;
		}
		else {
			return 18;
		}
	}

	std::uint32_t CustomSkillsManager::GetCurrentPerkPoints()
	{
		if (IsOurMenuMode()) {
			if (_menuSkills && _menuSkills->PerkPoints) {

				auto v = _menuSkills->PerkPoints->value;

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
			if (_menuSkills && _menuSkills->PerkPoints) {
				_menuSkills->PerkPoints->value = a_value;
				return;
			}
		}

		if (const auto player = RE::PlayerCharacter::GetSingleton()) {
			player->perkCount = a_value;
		}
	}

	void CustomSkillsManager::SetMenuState(MenuState a_state)
	{
		_menuState = a_state;
		UpdateVars();
	}

	void CustomSkillsManager::SetTrainingState(MenuState a_state)
	{
		_trainingState = a_state;
	}

	void CustomSkillsManager::SetBeastMode(bool a_beastMode)
	{
		REL::Relocation<bool*> isBeastMode{ REL::ID(RE::Offset::IsBeastMode) };
		*isBeastMode.get() = a_beastMode;
		UpdateVars();
	}

	bool CustomSkillsManager::IsOurMenuMode()
	{
		return _menuState != MenuState::None && _menuSkills != nullptr;
	}

	bool CustomSkillsManager::IsBeastMode()
	{
		REL::Relocation<bool*> isBeastMode{ RE::Offset::IsBeastMode };
		return *isBeastMode.get();
	}

	bool CustomSkillsManager::IsOurTrainingMode()
	{
		return _trainingState != MenuState::None && _trainingSkill != nullptr;
	}

	float CustomSkillsManager::GetSkillLevel(RE::ActorValue a_skill)
	{
		if (const auto skill = GetCurrentSkill(a_skill)) {
			return skill->GetLevel();
		}

		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetActorValue(a_skill);
	}

	float CustomSkillsManager::GetSkillProgressPercent(RE::ActorValue a_skill)
	{
		if (const auto skill = GetCurrentSkill(a_skill)) {
			return skill->Ratio ? skill->Ratio->value * 100.0f : 0.0f;
		}

		// WerewolfPerks / VampirePerks store skill progress
		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetActorValue(a_skill);
	}

	float CustomSkillsManager::GetBaseSkillLevel(RE::ActorValue a_skill)
	{
		if (const auto skill = GetCurrentSkill(a_skill)) {
			return skill->GetLevel();
		}

		const auto playerCharacter = RE::PlayerCharacter::GetSingleton();
		return playerCharacter->GetBaseActorValue(a_skill);
	}

	std::shared_ptr<SkillGroup> CustomSkillsManager::FindSkillMenu(const std::string& a_key)
	{
		if (const auto i = _groupIds.find(a_key); i != _groupIds.end()) {
			return i->second;
		}

		return nullptr;
	}

	std::shared_ptr<Skill> CustomSkillsManager::FindSkill(const std::string& a_key)
	{
		if (const auto i = _skillIds.find(a_key); i != _skillIds.end()) {
			const auto& [group, index] = i->second;
			return group->Skills[index];
		}

		return nullptr;
	}

	auto CustomSkillsManager::FindSkillOrigin(const std::string& a_key)
		-> std::pair<std::shared_ptr<SkillGroup>, std::size_t>
	{
		if (const auto i = _skillIds.find(a_key); i != _skillIds.end()) {
			return i->second;
		}

		return {};
	}

	std::shared_ptr<Skill> CustomSkillsManager::FindSkillFromGlobalLevel(RE::TESGlobal* a_global)
	{
		const auto it = _requirementSkills.find(a_global);
		return it != _requirementSkills.end() ? it->second : nullptr;
	}

	std::shared_ptr<Skill> CustomSkillsManager::GetCurrentSkill(RE::ActorValue a_value)
	{
		const std::uint32_t index = util::to_underlying(a_value) -
			util::to_underlying(RE::ActorValue::kTotal);

		if (_menuSkills && index < _menuSkills->Skills.size()) {
			return _menuSkills->Skills[index];
		}
		else if (_trainingSkill && index == 0) {
			return _trainingSkill;
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
		for (const auto& [key, group] : _groupIds) {
			if (!group)
				continue;

			if (group->OpenMenu) {
				const auto amt = static_cast<std::int16_t>(group->OpenMenu->value);
				if (amt > 0) {
					group->OpenMenu->value = 0;

					OpenStatsMenu(group);
					return;
				}
			}

			if (group->DebugReload) {
				if (group->DebugReload->value > 0) {
					group->DebugReload->value = 0;

					reload = true;
					break;
				}
			}

			for (const auto& sk : group->Skills) {
				if (sk && sk->ShowLevelup) {
					const auto amt = static_cast<std::int16_t>(sk->ShowLevelup->value);
					if (amt > 0) {
						sk->ShowLevelup->value = 0;

						Game::ShowSkillIncreasedMessage(sk->GetName(), amt);
						return;
					}
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
				_menuSkills = nullptr;
			}
			break;

		case MenuState::WaitingToClose:
			if (!IsStatsMenuOpen()) {
				closed = true;
				SetMenuState(MenuState::None);
				_menuSkills = nullptr;
			}
			break;
		}
	}

	void CustomSkillsManager::UpdateVars()
	{
		const bool modeOn = _menuState != MenuState::None && _menuSkills != nullptr;
		if (modeOn) {
			*IsSingleSkillMode = _menuSkills->Skills.size() <= 1;
			*UseBeastSkillInfo = _menuSkills->Skills.size() == 1 &&
				(_menuSkills->Skills[0]
					 ? _menuSkills->Skills[0]->Level == nullptr
					 : (_menuSkills->ActorValues[0] == RE::ActorValue::kWerewolfPerks ||
						_menuSkills->ActorValues[0] == RE::ActorValue::kVampirePerks));
			*CameraRightPoint = _menuSkills->CameraRightPoint;
		}
		else {
			const bool beastMode = IsBeastMode();
			*IsSingleSkillMode = beastMode;
			*UseBeastSkillInfo = beastMode;
			*CameraRightPoint = beastMode ? 2 : 1;
		}
	}

	void CustomSkillsManager::UpdateTraining()
	{
		if (RE::Main::GetSingleton()->freezeTime || RE::UI::GetSingleton()->GameIsPaused()) {
			return;
		}

		const bool isMenuOpen = RE::UI::GetSingleton()->IsMenuOpen(RE::TrainingMenu::MENU_NAME);
		if (isMenuOpen || !IsMenuControlsEnabled()) {
			return;
		}

		switch (_trainingState) {
		case MenuState::None:
			break;

		case MenuState::WaitingToOpen:
			if (isMenuOpen) {
				SetTrainingState(MenuState::Open);
			}
			break;

		case MenuState::Open:
			if (!isMenuOpen) {
				SetTrainingState(MenuState::None);
				_trainingSkill = nullptr;
				_trainingMax = 0;
			}
			break;

		case MenuState::WaitingToClose:
			if (!isMenuOpen) {
				SetTrainingState(MenuState::None);
				_trainingSkill = nullptr;
				_trainingMax = 0;
			}
			break;
		}
	}
}
