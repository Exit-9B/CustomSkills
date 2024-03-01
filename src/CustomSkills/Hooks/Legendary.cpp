#include "Legendary.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Legendary::WriteHooks()
	{
		ResetSkillLevelPatch();
		PlayerSkillsPatch();
		RefundPerksPatch();
	}

	void Legendary::ResetSkillLevelPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::LegendarySkillResetConfirmCallback::Run,
			0x1F6);

		REL::make_pattern<"8B 56 1C FF 50 20">().match_or_fail(hook.address());

		static auto LegendaryReset =
			+[](RE::ActorValueOwner* a_avOwner, RE::ActorValue a_skill, float a_resetValue)
		{
			if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_skill)) {
				skill->LegendaryReset(a_resetValue);
			}
			else {
				a_avOwner->SetBaseActorValue(a_skill, a_resetValue);
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr)
			{
				Xbyak::Label funcLbl;

				mov(edx, ptr[rsi + offsetof(RE::LegendarySkillResetConfirmCallback, skill)]);
				jmp(ptr[rip + funcLbl]);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(reinterpret_cast<std::uintptr_t>(LegendaryReset));
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_call<6>(hook.address(), patch->getCode());
	}

	void Legendary::PlayerSkillsPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::LegendarySkillResetConfirmCallback::Run,
			0x20D);

		using MakeLegendary_t = void(RE::PlayerCharacter::PlayerSkills*, RE::ActorValue);
		static REL::Relocation<MakeLegendary_t> _MakeLegendary;

		auto MakeLegendary = +[](RE::PlayerCharacter::PlayerSkills* a_playerSkills,
								 RE::ActorValue a_actorValue)
		{
			// We don't do this here for custom skills
			if (!CustomSkillsManager::IsOurMenuMode()) {
				_MakeLegendary(a_playerSkills, a_actorValue);
			}
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_MakeLegendary = trampoline.write_call<5>(hook.address(), MakeLegendary);
	}

	void Legendary::RefundPerksPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::BGSSkillPerkTreeNode::RefundPerks,
			0xED);

		REL::make_pattern<"44 00 B8 ?? ?? 00 00">().match_or_fail(hook.address());

		static auto ModifyPerkPoints = +[](std::uint8_t a_countDelta)
		{
			auto newCount = CustomSkillsManager::GetCurrentPerkPoints() + a_countDelta;
			if (newCount > 255) {
				newCount = 255;
			}

			CustomSkillsManager::SetCurrentPerkPoints(static_cast<std::uint8_t>(newCount));
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(cl, r15b);
				call(ptr[rip + funcLbl]);
				xor_(r8d, r8d);
				mov(rdx, r13);

				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(ModifyPerkPoints),
			hook.address() + 0x7);
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}
}
