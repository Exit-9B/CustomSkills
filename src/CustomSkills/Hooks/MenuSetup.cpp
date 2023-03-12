#include "MenuSetup.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void MenuSetup::WriteHooks()
	{
		MenuPropertiesPatch();
		SkillDomeArtPatch();
		CameraPatch();
		SkillArrayPatch();
		UpdateSkillPatch();
		CreateStarsPatch();
	}

	void MenuSetup::MenuPropertiesPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Create, 0x5D);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using StatsMenu_ctor_t = RE::StatsMenu* (*)(RE::StatsMenu*);
		static REL::Relocation<StatsMenu_ctor_t> _StatsMenu_ctor;

		auto SetupStatsMenu = +[](RE::StatsMenu* a_mem)
		{
			auto menu = _StatsMenu_ctor(a_mem);
			if (CustomSkillsManager::IsOurMenuMode()) {
				menu->selectedTree = 0;
				menu->numSelectableTrees = 2;
			}
			return menu;
		};

		auto& trampoline = SKSE::GetTrampoline();
		_StatsMenu_ctor = trampoline.write_call<5>(hook.address(), SetupStatsMenu);
	}

	void MenuSetup::SkillDomeArtPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Ctor, 0x413);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using RequestModelAsync_t = std::uint32_t(const char*, void**, void*, void*);
		static REL::Relocation<RequestModelAsync_t> _RequestModelAsync;

		auto RequestSkillDomeModel =
			+[](const char* a_path, void** a_id, void* a_params, void* a_arg4) -> std::uint32_t
		{
			auto path = a_path;
			if (CustomSkillsManager::IsOurMenuMode() &&
				!CustomSkillsManager::_menuSkill->Skydome.empty()) {
				path = CustomSkillsManager::_menuSkill->Skydome.c_str();
			}
			return _RequestModelAsync(path, a_id, a_params, a_arg4);
		};

		auto& trampoline = SKSE::GetTrampoline();
		_RequestModelAsync = trampoline.write_call<5>(hook.address(), RequestSkillDomeModel);
	}

	void MenuSetup::CameraPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetCameraTarget, 0x27E);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::IsUsingBeastNif);
	}

	void MenuSetup::SkillArrayPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x6B6);

		REL::make_pattern<
			"49 8B 4D 10 "
			"48 8B 01 "
			"44 89 74 24 30 "
			"C7 44 24 28 5A 00 00 00">()
			.match_or_fail(hook.address());

		static auto GetRequiredArraySize = +[]() -> std::uint32_t
		{
			return CustomSkillsManager::IsOurMenuMode() ? 5 : 90;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(GetRequiredArraySize));
				call(rax);
				mov(ptr[rsp + 0x28], eax);
				mov(rcx, ptr[r13 + 0x10]);
				mov(rax, ptr[rcx]);
				mov(ptr[rsp + 0x30], r14d);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x14);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		util::write_14branch(hook.address(), patch->getCode());
	}

	void MenuSetup::UpdateSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x90);

		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());

		static auto ShouldSkipSkillUpdate = +[](std::int32_t a_idx) -> bool
		{
			return CustomSkillsManager::IsOurMenuMode()
				? (a_idx > 0)
				: CustomSkillsManager::IsBeastMode();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				push(rcx);

				mov(ecx, r15d);
				mov(rax, reinterpret_cast<std::uintptr_t>(ShouldSkipSkillUpdate));
				call(rax);
				cmp(al, 0);

				pop(rcx);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x7);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void MenuSetup::CreateStarsPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::CreateStars, 0x92);

		using GetActorValueInfo_t = RE::ActorValueInfo*(RE::ActorValue);
		static REL::Relocation<GetActorValueInfo_t> _GetActorValueInfo;

		auto GetActorValueInfo = +[](RE::ActorValue a_actorValue) -> RE::ActorValueInfo*
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				if (a_actorValue != CustomSkillsManager::MENU_AV) {
					// Just query something that can't have a tree
					a_actorValue = RE::ActorValue::kHealth;
				}
			}

			return _GetActorValueInfo(a_actorValue);
		};

		auto& trampoline = SKSE::GetTrampoline();
		_GetActorValueInfo = trampoline.write_call<5>(hook.address(), GetActorValueInfo);
	}
}
