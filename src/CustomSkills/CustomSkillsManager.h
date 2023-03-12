#pragma once

#include "Skill.h"

namespace CustomSkills
{
	class CustomSkillsManager final
	{
	public:
		static void Initialize();
		static void LoadSkills();

	public:
		enum class MenuState : uint8_t
		{
			None = 0,
			WaitingToOpen = 1,
			Open = 2,
			WaitingToClose = 3,
		};

		static void ShowLevelup(std::string_view a_name, std::int32_t a_level);

		static void CloseStatsMenu();

		static void OpenStatsMenu(std::shared_ptr<Skill> a_skill);

		static bool IsMenuControlsEnabled();

		static bool IsStatsMenuOpen();

		static std::uint32_t GetCurrentPerkPoints();

		static void SetCurrentPerkPoints(std::uint8_t a_value);

		static void SetMenuState(MenuState a_state);

		static void SetBeastMode(bool a_beastMode);

		static bool IsOurMenuMode();

		static bool IsBeastMode();

		static float GetSkillLevel(RE::ActorValue a_skill);

		static float GetSkillProgressPercent(RE::ActorValue a_skill);

		static float GetBaseSkillLevel(RE::ActorValue a_skill);

		static std::shared_ptr<Skill> FindSkill(const std::string& a_key);

		static std::shared_ptr<Skill> FindSkillFromGlobalLevel(RE::TESGlobal* a_global);

		static void UpdateSkills();

		static void UpdateMenu();

		static void UpdateVars();

		inline static constexpr auto MENU_AV = RE::ActorValue::kEnchanting;
		inline static constexpr auto MENU_NAME = RE::StatsMenu::MENU_NAME;

		inline static REL::Relocation<bool*> IsSingleSkillMode;
		inline static REL::Relocation<bool*> IsUsingBeastNif;
		inline static REL::Relocation<bool*> ShouldHideLevel;

		inline static RE::BGSSkillPerkTreeNode* _originalSkillTree = nullptr;
		inline static std::uint32_t _originalSkillTreeWidth = 3;

		inline static std::vector<std::shared_ptr<Skill>> _skills;
		inline static std::map<std::string, std::shared_ptr<Skill>> _skillIds;

		inline static std::shared_ptr<Skill> _menuSkill = nullptr;
		inline static MenuState _menuState = MenuState::None;
		inline static RE::BSFixedString _colorOfSkillNormal = "#FFFFFF";
	};
}
