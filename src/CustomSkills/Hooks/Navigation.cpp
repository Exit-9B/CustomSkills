#include "Navigation.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Navigation::WriteHooks()
	{
		LockRotationPatch();
		RotationSpeedPatch();
		SelectedTreePatch();
	}

	void Navigation::LockRotationPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Rotate, 0x46);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::IsSingleSkillMode);
	}

	void Navigation::RotationSpeedPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Rotate, 0x35A);
		REL::make_pattern<"F3 0F 59 F2 41 0F 2F F0">().match_or_fail(hook.address());

		auto ModRotationSpeed = +[](std::uint32_t numTrees, float t) -> float
		{
			return t * 18.0f / numTrees;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu, numSelectableTrees)]);
				movaps(xmm1, xmm2);
				call(ptr[rip + funcLbl]);
				mulss(xmm6, xmm0);
				comiss(xmm6, xmm8);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(ModRotationSpeed),
			hook.address() + 0x8);
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x8);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Navigation::SelectedTreePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::UpdateSelectedTree,
			0x5E);
		REL::make_pattern<"8B 87 10 03 00 00">().match_or_fail(hook.address());

		auto ComputeSelectedTree = +[](RE::StatsMenu* menu, float angle) -> std::uint32_t
		{
			const float skillAngle = 360.0f / menu->numSelectableTrees;

			angle += skillAngle * 0.5f;

			if (angle >= 360.0f) {
				angle = 0.0f;
			}

			return static_cast<std::uint32_t>(angle / skillAngle);
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				movaps(xmm1, xmm2);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				mov(ebx, eax);
				jmp(ptr[rip + retnLbl]);
				nop();

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		Patch
			patch{ reinterpret_cast<std::uintptr_t>(ComputeSelectedTree), hook.address() + 0xFA };
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}
}
