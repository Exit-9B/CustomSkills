#include "SkillInfo.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Game.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void SkillInfo::WriteHooks()
	{
		SkillStatsPatch();
		SkillDescriptionPatch();
	}

	static void SetSkillStats(RE::StatsMenu* a_menu)
	{
		const auto numSkills = CustomSkillsManager::GetCurrentSkillCount();
		auto skillStats = std::vector<RE::GFxValue>(5 * numSkills);

		static auto iDifficultyLevelMax = RE::GameSettingCollection::GetSingleton()->GetSetting(
			"iDifficultyLevelMax");
		const bool legendaryAvailable = iDifficultyLevelMax && iDifficultyLevelMax->GetSInt() >= 5;

		for (std::uint32_t i = 0; i < skillStats.size(); i += 5) {
			RE::GFxValue& level = skillStats[i + 0];
			RE::GFxValue& name = skillStats[i + 1];
			RE::GFxValue& percent = skillStats[i + 2];
			RE::GFxValue& color = skillStats[i + 3];
			RE::GFxValue& legendary = skillStats[i + 4];

			const RE::ActorValue actorValue = a_menu->skillTrees[i / 5];
			if (const auto skill = CustomSkillsManager::GetCurrentSkill(actorValue)) {
				if (skill->Level) {
					level.SetNumber(static_cast<std::uint32_t>(skill->Level->value));
				}
				else {
					level.SetString(""sv);
				}

				name.SetString(skill->GetName());
				percent.SetNumber(skill->GetProgressPercent());

				if (skill->UpdateColor()) {
					color.SetString(skill->ColorStr);
				}
				else {
					color.SetString(CustomSkillsManager::_colorOfSkillNormal);
				}

				legendary.SetNumber(
					legendaryAvailable && skill->Legendary
						? static_cast<std::uint32_t>(skill->Legendary->value)
						: 0.0);
			}
			else {
				name.SetString(Game::GetActorValueName(actorValue));
				color.SetString(Game::GetActorValueColor(actorValue));

				const auto player = RE::PlayerCharacter::GetSingleton();
				const auto playerSkills = player ? player->skills : nullptr;
				const std::size_t idx = util::to_underlying(actorValue) - 6;
				if (playerSkills && idx < 18) {
					const auto& data = playerSkills->data->skills[idx];
					level.SetNumber(player->GetActorValue(actorValue));
					percent.SetNumber((data.xp / data.levelThreshold) * 100);

					legendary.SetNumber(
						legendaryAvailable ? playerSkills->data->legendaryLevels[idx] : 0);
				}
				else if (
					actorValue == RE::ActorValue::kWerewolfPerks ||
					actorValue == RE::ActorValue::kVampirePerks) {
					level.SetString(""sv);
					percent.SetNumber(player->GetActorValue(actorValue));
				}
			}
		}

		a_menu->uiMovie->SetVariableArray(
			RE::GFxMovie::SetArrayType::kValue,
			"StatsMenu.SkillStatsA",
			0,
			skillStats.data(),
			static_cast<std::uint32_t>(skillStats.size()),
			RE::GFxMovie::SetVarType::kNormal);
	}

	void SkillInfo::SkillStatsPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x9D);
		REL::make_pattern<"0F 85 A7 03 00 00">().match_or_fail(hook.address() - 0x6);

		auto retn = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x6EB);

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(rcx, r13);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);
				nop();

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(&SetSkillStats), retn.address() };
		patch.ready();

		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void SkillInfo::SkillDescriptionPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x154D);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using GetDescription_t = void(RE::BSString&, RE::ActorValue);
		static REL::Relocation<GetDescription_t> _GetDescription;

		auto GetDescription = +[](RE::BSString& a_description, RE::ActorValue a_actorValue)
		{
			if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
				a_description = skill->Description;
			}
			else {
				_GetDescription(a_description, a_actorValue);
			}
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetDescription = trampoline.write_call<5>(hook.address(), GetDescription);
	}
}
