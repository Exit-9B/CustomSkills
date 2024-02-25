#include "MenuSetup.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Game.h"
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
		SkillTreePatch();
		CloseMenuPatch();
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
				menu->selectedTree = CustomSkillsManager::_menuSkills->LastSelectedTree;
				menu->numSelectableTrees = 2;

				menu->skillTrees.clear();
				const auto numSkills = (std::max)(
					18ULL,
					CustomSkillsManager::_menuSkills->Skills.size());
				for (int i = 0; i < numSkills; ++i) {
					menu->skillTrees.push_back(static_cast<RE::ActorValue>(
						util::to_underlying(RE::ActorValue::kTotal) + i));
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

		using RequestModelAsync_t = std::uint32_t(const char*, void**, void*, void*);
		static REL::Relocation<RequestModelAsync_t> _RequestModelAsync;

		auto RequestSkillDomeModel =
			+[](const char* a_path, void** a_id, void* a_params, void* a_arg4) -> std::uint32_t
		{
			auto path = a_path;
			if (CustomSkillsManager::IsOurMenuMode() &&
				!CustomSkillsManager::_menuSkills->Skydome.empty()) {
				path = CustomSkillsManager::_menuSkills->Skydome.c_str();
			}
			return _RequestModelAsync(path, a_id, a_params, a_arg4);
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
			return CustomSkillsManager::GetCurrentSkillCount() * 5;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				call(ptr[rip + funcLbl]);
				mov(ptr[rsp + 0x28], eax);
				mov(rcx, ptr[r13 + 0x10]);
				mov(rax, ptr[rcx]);
				mov(ptr[rsp + 0x30], r14d);

				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(GetRequiredArraySize),
			hook.address() + 0x14);
		patch->ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x14);
		util::write_14branch(hook.address(), patch->getCode());
	}

	void MenuSetup::UpdateSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::UpdateSkillList, 0x69B);

		REL::make_pattern<"49 83 C4 04 41 83 FF 12">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				add(r12, 4);
				call(ptr[rip + funcLbl]);
				cmp(r15d, eax);

				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetCurrentSkillCount),
			hook.address() + 0x8);
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x8);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void MenuSetup::SkillTreePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::GetActorValueInfo, 0x18);
		REL::make_pattern<"33 C0 C3 CC CC CC">().match_or_fail(hook.address());

		auto GetActorValueInfo = +[](std::uint32_t a_actorValue) -> RE::ActorValueInfo*
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				const std::uint32_t index = a_actorValue -
					util::to_underlying(RE::ActorValue::kTotal);

				if (index < CustomSkillsManager::_menuSkills->Skills.size()) {
					return CustomSkillsManager::_menuSkills->Skills[index]->Info;
				}
				return Game::GetActorValueInfo(RE::ActorValue::kHealth);
			}
			return nullptr;
		};

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), GetActorValueInfo);
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

		auto SaveLastSelectedTree = +[](RE::StatsMenu* a_statsMenu)
		{
			if (CustomSkillsManager::IsBeastMode()) {
				return;
			}
			else if (CustomSkillsManager::IsOurMenuMode()) {
				CustomSkillsManager::_menuSkills->LastSelectedTree = a_statsMenu->selectedTree;
			}
			else {
				REL::Relocation<std::uint32_t*> lastSelectedTree{ REL::ID(383192) };
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
