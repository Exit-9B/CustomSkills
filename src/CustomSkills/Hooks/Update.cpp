#include "Update.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Update::WriteHooks()
	{
		FrameHook();
		SetBeastModePatch();
		ExitModePatch();
	}

	void Update::FrameHook()
	{
		auto& trampoline = SKSE::GetTrampoline();

		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Main::Update, 0x3E);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		_DoFrame = trampoline.write_call<5>(hook.address(), &DoFrame);
	}

	void Update::SetBeastModePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::OpenStatsMenu, 0x63);
		REL::make_pattern<"88 1D">().match_or_fail(hook.address());

		static auto SetBeastMode = +[](bool a_beastMode)
		{
			CustomSkillsManager::SetBeastMode(a_beastMode);
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(r12, rcx);
				mov(r13, rdx);

				mov(cl, bl);
				mov(rax, reinterpret_cast<std::uintptr_t>(SetBeastMode));
				call(rax);

				xor_(r9d, r9d);
				mov(rcx, r12);
				mov(rdx, r13);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x6);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Update::ExitModePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>{ RE::Offset::StatsMenu::DtorImpl, 0x236 };
		REL::make_pattern<"44 88 2D">().match_or_fail(hook.address());

		static auto ExitMode = +[]()
		{
			CustomSkillsManager::SetBeastMode(false);
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(ExitMode));
				call(rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x7);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Update::DoFrame(RE::Main* a_main)
	{
		CustomSkillsManager::UpdateMenu();
		CustomSkillsManager::UpdateSkills();
		return _DoFrame(a_main);
	}
}
