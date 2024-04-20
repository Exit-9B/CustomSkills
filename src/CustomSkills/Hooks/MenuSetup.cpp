#include "MenuSetup.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Game.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void MenuSetup::WriteHooks()
	{
		MenuConstructorPatch();
		SkillDomeArtPatch();
		CameraPatch();
		SkillTreePatch();
		CreateStarsPatch();
		CloseMenuPatch();
	}

	void MenuSetup::MenuConstructorPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Create, 0x5D);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using StatsMenu_ctor_t = RE::StatsMenu* (RE::StatsMenu*);
		static REL::Relocation<StatsMenu_ctor_t> _StatsMenu_ctor;

		auto SetupStatsMenu = +[](RE::StatsMenu* a_mem)
		{
			CustomSkillsManager::NotifyOpeningSkills();

			const auto menu = _StatsMenu_ctor(a_mem);

			if (CustomSkillsManager::IsOurMenuMode()) {
				const std::uint32_t numSkills = static_cast<std::uint32_t>(
					CustomSkillsManager::_menuSkills->Skills.size());

				menu->selectedTree = CustomSkillsManager::_menuSkills->LastSelectedTree;
				menu->numSelectableTrees = (std::max)(2U, numSkills);

				menu->skillTrees.clear();
				for (std::uint32_t i = 0; i < numSkills; ++i) {
					menu->skillTrees.push_back(CustomSkillsManager::_menuSkills->ActorValues[i]);
				}
				for (std::uint32_t i = numSkills; i < 18; ++i) {
					menu->skillTrees.push_back(
						static_cast<RE::ActorValue>(CUSTOM_SKILL_BASE_VALUE + i));
				}
			}
			return menu;
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_StatsMenu_ctor = trampoline.write_call<5>(hook.address(), SetupStatsMenu);
	}

	void MenuSetup::SkillDomeArtPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Ctor, 0x413);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using RequestModelAsync_t = RE::BSResource::ErrorCode(
			const char*,
			RE::BSResource::ID**,
			const void*,
			std::uint32_t);
		static REL::Relocation<RequestModelAsync_t> _RequestModelAsync;

		auto RequestSkillDomeModel =
			+[](const char* a_path,
				RE::BSResource::ID** a_handle,
				const void* a_params,
				std::uint32_t a_arg4) -> RE::BSResource::ErrorCode
		{
			auto path = a_path;
			if (CustomSkillsManager::IsOurMenuMode() &&
				!CustomSkillsManager::_menuSkills->Skydome.empty()) {
				path = CustomSkillsManager::_menuSkills->Skydome.c_str();
			}
			return _RequestModelAsync(path, a_handle, a_params, a_arg4);
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_RequestModelAsync = trampoline.write_call<5>(hook.address(), RequestSkillDomeModel);
	}

	void MenuSetup::CameraPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetCameraTarget, 0x27E);
		REL::make_pattern<
			"80 3D ?? ?? ?? ?? 00 "
			"BA 02 00 00 00 "
			"75 05 "
			"BA 01 00 00 00">()
			.match_or_fail(hook.address());

#pragma pack(push, 1)
		struct Assembly
		{
			// mov r16, r/m32
			std::uint8_t mov;    // 0 - 0x8B
			std::uint8_t modrm;  // 1 - 0x15
			std::int32_t disp;   // 2
		};
		static_assert(offsetof(Assembly, mov) == 0x0);
		static_assert(offsetof(Assembly, modrm) == 0x1);
		static_assert(offsetof(Assembly, disp) == 0x2);
		static_assert(sizeof(Assembly) == 0x6);
#pragma pack(pop)

		const auto var = CustomSkillsManager::CameraRightPoint.get();
		const auto rip = hook.address() + sizeof(Assembly);
		const auto disp = reinterpret_cast<const std::byte*>(var) -
			reinterpret_cast<const std::byte*>(rip);

		// mov edx, dword ptr [rip+offset]
		Assembly mem{
			.mov = 0x8B,
			.modrm = 0x15,
			.disp = static_cast<std::int32_t>(disp),
		};

		REL::safe_fill(hook.address(), REL::NOP, 0x13);
		REL::safe_write(hook.address(), std::addressof(mem), sizeof(mem));
	}

	void MenuSetup::SkillTreePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::GetActorValueInfo, 0x18);
		REL::make_pattern<"33 C0 C3 CC CC CC">().match_or_fail(hook.address());

		auto GetActorValueInfo = +[](RE::ActorValue a_actorValue) -> RE::ActorValueInfo*
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
					return skill->Info;
				}
				return Game::GetActorValueInfo(RE::ActorValue::kHealth);
			}
			return nullptr;
		};

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), GetActorValueInfo);
	}

	void MenuSetup::CreateStarsPatch()
	{
		auto hook1 = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessMessage, 0x9A3);
		auto hook2 = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessMessage, 0xCF8);
		REL::make_pattern<"83 BE D0 01 00 00 18">().match_or_fail(hook1.address());
		REL::make_pattern<"83 BE D0 01 00 00 18">().match_or_fail(hook2.address());

		auto GetMaxSkillIndex = +[]() -> std::uint32_t
		{
			return CustomSkillsManager::GetCurrentSkillCount() + 6;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				call(ptr[rip + funcLbl]);
				cmp(dword[rsi + 0x1D0], eax);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		// TRAMPOLINE: 16
		auto& trampoline = SKSE::GetTrampoline();

		auto patch1 = new Patch(
			reinterpret_cast<std::uintptr_t>(GetMaxSkillIndex),
			hook1.address() + 0x7);
		patch1->ready();

		REL::safe_fill(hook1.address(), REL::NOP, 0x7);
		trampoline.write_branch<6>(hook1.address(), patch1->getCode());

		auto patch2 = new Patch(
			reinterpret_cast<std::uintptr_t>(GetMaxSkillIndex),
			hook2.address() + 0x7);
		patch2->ready();

		REL::safe_fill(hook2.address(), REL::NOP, 0x7);
		trampoline.write_branch<6>(hook2.address(), patch2->getCode());
	}

	void MenuSetup::CloseMenuPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::DtorImpl, 0x39);
		REL::make_pattern<
			"80 3D ?? ?? ?? ?? 00 "
			"75 0C "
			"8B 81 C0 01 00 00 "
			"89 05">()
			.match_or_fail(hook.address());

		auto SaveLastSelectedTree = +[](const RE::StatsMenu* a_statsMenu)
		{
			static REL::Relocation<std::uint32_t*> lastSelectedTree{
				RE::Offset::StatsMenu::LastSelectedTree
			};

			if (CustomSkillsManager::IsBeastMode()) {
				return;
			}
			else if (CustomSkillsManager::IsOurMenuMode()) {
				CustomSkillsManager::_menuSkills->LastSelectedTree = a_statsMenu->selectedTree;
				if (CustomSkillsManager::FindSkillMenu("SKILLS"sv) ==
					CustomSkillsManager::_menuSkills) {
					*lastSelectedTree = a_statsMenu->selectedTree;
				}
			}
			else {
				*lastSelectedTree = a_statsMenu->selectedTree;
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x15)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				call(ptr[rip + funcLbl]);
				mov(rcx, rbx);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(SaveLastSelectedTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x15);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}
}
