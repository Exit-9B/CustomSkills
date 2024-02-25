#include "SkillInfo.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void SkillInfo::WriteHooks()
	{
		SkillNamePatch();
		SkillColorPatch();
		SkillDescriptionPatch();
	}

	void SkillInfo::SkillNamePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x1EF);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using GetSkillName_t = const char*(RE::ActorValue);
		static REL::Relocation<GetSkillName_t> _GetSkillName;

		auto GetSkillName = +[](RE::ActorValue a_skill) -> const char*
		{
			if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_skill)) {
				return skill->GetName().data();
			}
			return _GetSkillName(a_skill);
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetSkillName = trampoline.write_call<5>(hook.address(), GetSkillName);
	}

	void SkillInfo::SkillColorPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x2F1);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using GetSkillColor_t = const char*(RE::ActorValue);
		static REL::Relocation<GetSkillColor_t> _GetSkillColor;

		auto GetSkillColor = +[](RE::ActorValue a_skill) -> const char*
		{
			if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_skill)) {
				if (skill->UpdateColor()) {
					return skill->ColorStr.c_str();
				}
				else {
					return CustomSkillsManager::_colorOfSkillNormal.c_str();
				}
			}

			return _GetSkillColor(a_skill);
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetSkillColor = trampoline.write_call<5>(hook.address(), GetSkillColor);
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
