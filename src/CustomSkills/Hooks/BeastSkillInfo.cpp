#include "BeastSkillInfo.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void BeastSkillInfo::WriteHooks()
	{
		BeastSkillPatch();
		ZoomOutPatch();
	}

	void BeastSkillInfo::BeastSkillPatch()
	{
		auto hook1 = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x90);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook1.address());

		util::write_disp(
			hook1.address() + 0x2,
			hook1.address() + 0x7,
			CustomSkillsManager::UseBeastSkillInfo);

		auto hook2 = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::UpdateSkillList,
			0x6A9);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook2.address());

		util::write_disp(
			hook2.address() + 0x2,
			hook2.address() + 0x7,
			CustomSkillsManager::UseBeastSkillInfo);
	}

	void BeastSkillInfo::ZoomOutPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x5A6);

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::UseBeastSkillInfo);
	}
}
