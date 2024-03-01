#include "BeastSkillInfo.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void BeastSkillInfo::WriteHooks()
	{
		BeastSkillPatch();
		SkillNamePatch();
		ZoomOutPatch();
		PerkViewPatch();
	}

	void BeastSkillInfo::BeastSkillPatch()
	{
		auto hook1 = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x90);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook1.address());

		util::write_disp(
			hook1.address() + 0x2,
			hook1.address() + 0x7,
			CustomSkillsManager::ShouldHideLevel);

		auto hook2 = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x6A9);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook2.address());

		util::write_disp(
			hook2.address() + 0x2,
			hook2.address() + 0x7,
			CustomSkillsManager::ShouldHideLevel);
	}

	void BeastSkillInfo::SkillNamePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::SetBeastSkillInfo,
			0x20C);

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

	void BeastSkillInfo::ZoomOutPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x5A6);

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::ShouldHideLevel);
	}

	void BeastSkillInfo::PerkViewPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x108F);

		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());
		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::ShouldHideLevel);
	}
}
