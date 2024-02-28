#include "Constellation.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Constellation::WriteHooks()
	{
		LoadSkydomePatch1();
		EnterConstellationPatch1();
		EnterConstellationPatch2();
		ExitConstellationPatch1();
		ExitConstellationPatch2();
		KinectPatch();
		UpdateConstellationPatch();
	}

	void Constellation::LoadSkydomePatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::LoadSkydome, 0x168);
		REL::make_pattern<"48 89 8C DF 98 00 00 00">().match_or_fail(hook.address());

		auto SetCImageShader = +[](RE::BSShaderProperty* shader, std::uint32_t index)
		{
			CustomSkillsManager::_cImageControllers[index].SetShader(shader);
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x28)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(edx, esi);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x16);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(SetCImageShader) };

		REL::safe_fill(hook.address(), REL::NOP, 0x28);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::EnterConstellationPatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Rotate, 0x3C6);

		auto EnterTree = +[](std::uint32_t index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[index];
			controller.Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x18)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(EnterTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x18);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::EnterConstellationPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Animate, 0x14B);

		auto EnterTree = +[](std::uint32_t index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[index];
			controller.Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x22)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rbx + offsetof(RE::StatsMenu, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(EnterTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x22);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::ExitConstellationPatch1()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x141);

		auto ExitTree = +[](std::uint32_t index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[index];
			if (controller.shader && !*CustomSkillsManager::IsSingleSkillMode) {
				controller.Exit();
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x5C)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x46, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(ExitTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x5C);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::ExitConstellationPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x2A1);

		auto ExitTree = +[](std::uint32_t index)
		{
			auto& controller = CustomSkillsManager::_cImageControllers[index];
			if (controller.shader) {
				controller.Exit();
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x4A)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, dword[rdi + offsetof(RE::StatsMenu, selectedTree)]);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x34, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(ExitTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x4A);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::KinectPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::GotoNode, 0xB9);

		auto SetSelectedTree = +[](RE::StatsMenu* statsMenu, std::uint32_t newIndex)
		{
			std::uint32_t oldIndex = statsMenu->selectedTree;
			auto& controllers = CustomSkillsManager::_cImageControllers;

			controllers[oldIndex].Exit();

			statsMenu->selectedTree = newIndex;
			controllers[newIndex].Enter();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x6B)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(rcx, rdi);
				mov(edx, r13d);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				nop(0x55, false);
				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(SetSelectedTree) };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0x6B);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Constellation::UpdateConstellationPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessMessage, 0x10B0);

		auto UpdateConstellation = +[](std::uint32_t index)
		{
			CustomSkillsManager::_cImageControllers[index].Update();
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
				: Xbyak::CodeGenerator(0xB2)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, edi);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		Patch
			patch{ reinterpret_cast<std::uintptr_t>(UpdateConstellation), hook.address() + 0xB2 };
		patch.ready();

		REL::safe_fill(hook.address(), REL::NOP, 0xB2);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}
}
