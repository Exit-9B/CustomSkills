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
		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();

		using DoFrame_t = void(RE::Main::*)();
		static REL::Relocation<DoFrame_t> _DoFrame;

		auto DoFrame = +[](RE::Main* a_main)
		{
			CustomSkillsManager::UpdateMenu();
			CustomSkillsManager::UpdateSkills();
			CustomSkillsManager::UpdateTraining();
			return _DoFrame(a_main);
		};

		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::Main::OnIdle, 0x3E);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		_DoFrame = trampoline.write_call<5>(hook.address(), DoFrame);
	}

	void Update::SetBeastModePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::OpenStatsMenu, 0x63);
		REL::make_pattern<"88 1D">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(r12, rcx);
				mov(r13, rdx);

				mov(cl, bl);
				call(ptr[rip + funcLbl]);

				xor_(r9d, r9d);
				mov(rcx, r12);
				mov(rdx, r13);

				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::SetBeastMode),
			hook.address() + 0x6);
		patch->ready();

		// TRAMPOLINE: 8
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

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_call<6>(hook.address(), ExitMode);
	}
}
