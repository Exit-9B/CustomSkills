#include "Legendary.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Legendary::WriteHooks()
	{
		LegendaryHintPatch();
		ProcessButtonPatch();
		ProcessMessagePatch();
		LegendarySkillConfirmPatch();
		ResetSkillLevelPatch();
		PlayerSkillsPatch();
		LegendaryAvailablePatch();
		RefundPerksPatch();
	}

	void Legendary::LegendaryHintPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x153);

		REL::make_pattern<
			"48 8B 0D ?? ?? ?? ?? "
			"48 81 C1 ?? 00 00 00 "
			"48 8B 01 "
			"41 8B D7 "
			"FF 50 18">()
			.match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr) : Xbyak::CodeGenerator(0x17)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retn;

				mov(ecx, r15d);
				call(ptr[rip + funcLbl]);
				jmp(retn);

				L(funcLbl);
				dq(a_funcAddr);

				L(retn);
			}
		};

		Patch patch{ reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetBaseSkillLevel) };
		patch.ready();

		assert(patch.getSize() <= 0x17);

		REL::safe_fill(hook.address(), REL::NOP, 0x17);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Legendary::ProcessButtonPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessButton, 0x14E);

		REL::make_pattern<
			"8B D0 "
			"48 8D 8F ?? 00 00 00 "
			"FF 53 18">()
			.match_or_fail(hook.address());

		static auto GetSkillLevelForLegendaryReset = +[](RE::ActorValue a_actorValue) -> float
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				if (CustomSkillsManager::_menuSkill->Level &&
					CustomSkillsManager::_menuSkill->Legendary) {
					return CustomSkillsManager::_menuSkill->Level->value;
				}
			}
			else {
				if (const auto player = RE::PlayerCharacter::GetSingleton()) {
					return player->GetBaseActorValue(a_actorValue);
				}
			}

			return 0.0f;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, eax);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch{
			reinterpret_cast<std::uintptr_t>(GetSkillLevelForLegendaryReset),
			hook.address() + 0xC
		};
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0xC);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Legendary::ProcessMessagePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessMessage, 0x4D0);

		REL::make_pattern<
			"E8 ?? ?? ?? ?? "
			"8B D0 "
			"48 8D 8F ?? 00 00 00 "
			"FF 53 18">()
			.match_or_fail(hook.address());

		using GetActorValue_t = RE::ActorValue(RE::StatsMenu*, std::uint32_t);
		static REL::Relocation<GetActorValue_t> _GetActorValue;

		auto GetActorValue = +[](RE::StatsMenu* a_menu, std::uint32_t a_selectedIndex) -> float
		{
			if (CustomSkillsManager::IsOurMenuMode()) {
				return CustomSkillsManager::_menuSkill->GetLevel();
			}
			else {
				auto actorValue = _GetActorValue(a_menu, a_selectedIndex);
				return RE::PlayerCharacter::GetSingleton()->GetBaseActorValue(actorValue);
			}
		};

		REL::safe_fill(hook.address() + 0x5, REL::NOP, 0xC);
		auto& trampoline = SKSE::GetTrampoline();
		_GetActorValue = trampoline.write_call<5>(hook.address(), GetActorValue);
	}

	void Legendary::LegendarySkillConfirmPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::LegendarySkillResetConfirmCallback::Run,
			0x1C0);

		REL::make_pattern<
			"48 81 C1 ?? 00 00 00 "
			"48 8B 01 "
			"8B 56 1C "
			"FF 50 18">()
			.match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				mov(ecx, ptr[rsi + 0x1C]);
				call(ptr[rip + funcLbl]);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetBaseSkillLevel),
			hook.address() + 0x10);
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x10);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
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
			if (CustomSkillsManager::IsOurMenuMode()) {
				CustomSkillsManager::_menuSkill->LegendaryReset(a_resetValue);
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

		auto& trampoline = SKSE::GetTrampoline();
		_MakeLegendary = trampoline.write_call<5>(hook.address(), MakeLegendary);
	}

	void Legendary::LegendaryAvailablePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::IsLegendaryDifficultyAvailable);

		REL::make_pattern<"83 3D">().match_or_fail(hook.address());

		static auto IsLegendaryAvailable = +[]()
		{
			// Yes, the game does this
			static auto
				iDifficultyLevelMax = RE::GameSettingCollection::GetSingleton()->GetSetting(
					"iDifficultyLevelMax");

			if (!iDifficultyLevelMax || iDifficultyLevelMax->GetSInt() < 5) {
				return false;
			}

			if (CustomSkillsManager::IsOurMenuMode()) {
				return CustomSkillsManager::_menuSkill->Legendary != nullptr;
			}
			else {
				return true;
			}
		};

		REL::safe_fill(hook.address(), REL::INT3, 0x20);
		util::write_14branch(hook.address(), IsLegendaryAvailable);
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

		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}
}
