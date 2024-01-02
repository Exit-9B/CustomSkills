#include "BeastSkillInfo.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void BeastSkillInfo::WriteHooks()
	{
		BeastSkillPatch();
		SkillProgressPatch();
		SkillNamePatch();
		ZoomOutPatch();
		PerkViewPatch();
		PerkSkillNamePatch();
	}

	void BeastSkillInfo::BeastSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x6A9);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());
		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::ShouldHideLevel);
	}

	void BeastSkillInfo::SkillProgressPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::SetBeastSkillInfo,
			0x16A);

		REL::make_pattern<
			"48 8B 0D ?? ?? ?? ?? "
			"48 81 C1 ?? 00 00 00 "
			"4C 8B 01 "
			"8B D0 "
			"41 FF 50 08">()
			.match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t hookAddress)
			{
				mov(ecx, eax);
				mov(rax,
					reinterpret_cast<std::uintptr_t>(
						&CustomSkillsManager::GetSkillProgressPercent));
				call(rax);
				jmp(ptr[rip]);
				dq(hookAddress + 0x17);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void BeastSkillInfo::SkillNamePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::SetBeastSkillInfo,
			0x20C);

		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using GetSkillName_t = const char*(std::uint32_t);
		static REL::Relocation<GetSkillName_t> _GetSkillName;

		auto GetSkillName = +[](std::uint32_t a_skill) -> const char*
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				if (!CustomSkillsManager::_menuSkill->Name.empty()) {
					return CustomSkillsManager::_menuSkill->Name.c_str();
				}
			}
			return _GetSkillName(a_skill);
		};

		auto& trampoline = SKSE::GetTrampoline();
		_GetSkillName = trampoline.write_call<5>(hook.address(), GetSkillName);
	}

	void BeastSkillInfo::ZoomOutPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Navigate, 0x5A6);
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

	void BeastSkillInfo::PerkSkillNamePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x10A0);
		REL::make_pattern<"48 83 C1 20 48 8B 01 FF 50 28">().match_or_fail(hook.address());

		static auto GetSkillName = +[](RE::ActorValueInfo* a_avInfo) -> const char*
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				return CustomSkillsManager::_menuSkill->Name.c_str();
			}

			return a_avInfo->GetFullName();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(GetSkillName));
				call(rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0xA);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}
}
