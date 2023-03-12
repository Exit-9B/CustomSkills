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
			Patch()
			{
				mov(ecx, r15d);
				mov(rax,
					reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetBaseSkillLevel));
				call(rax);
				nop(0x8);
			}
		};

		Patch patch{};
		patch.ready();
		assert(patch.getSize() == 0x17);

		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Legendary::ProcessButtonPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::ProcessButton, 0x14E);

		REL::make_pattern<
			"8B D0 "
			"48 8D 8F ?? 00 00 00 "
			"FF 53 18 "
			"0F 2F 05 ?? ?? ?? ?? "
			"0F 82 5A 09 00 00">()
			.match_or_fail(hook.address());


		static auto GetSkillLevelForLegendaryReset = +[](RE::ActorValue a_actorValue)
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
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(ecx, eax);
				mov(rax, reinterpret_cast<std::uintptr_t>(&GetSkillLevelForLegendaryReset));
				call(rax);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0xC);
			}
		};

		auto patch = new Patch(hook.address());
		patch->ready();

		util::write_14branch(hook.address(), patch->getCode());
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
			Patch()
			{
				mov(ecx, ptr[rsi + 0x1C]);
				mov(rax,
					reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetBaseSkillLevel));
				call(rax);
				nop(0x1);
			}
		};

		Patch patch{};
		patch.ready();
		assert(patch.getSize() == 0x10);

		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Legendary::ResetSkillLevelPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::LegendarySkillResetConfirmCallback::Run,
			0x1EB);

		REL::make_pattern<
			"48 8B 01 "
			"F3 0F 10 15 ?? ?? ?? ?? "
			"8B 56 1C "
			"FF 50 20">()
			.match_or_fail(hook.address());

		static auto ResetSkillLevel = +[](RE::ActorValue a_skill)
		{
			static auto
				fLegendarySkillResetValue = RE::GameSettingCollection::GetSingleton()->GetSetting(
					"fLegendarySkillResetValue");

			if (CustomSkillsManager::IsOurMenuMode()) {
				CustomSkillsManager::_menuSkill->LegendaryReset(
					fLegendarySkillResetValue->GetFloat());
			}
			else {
				RE::PlayerCharacter::GetSingleton()->SetBaseActorValue(
					a_skill,
					fLegendarySkillResetValue->GetFloat());
			}
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch()
			{
				mov(ecx, ptr[rsi + 0x1C]);
				mov(rax, reinterpret_cast<std::uintptr_t>(ResetSkillLevel));
				call(rax);
				nop(0x2);
			}
		};

		Patch patch{};
		patch.ready();
		assert(patch.getSize() == 0x11);
	}

	void Legendary::PlayerSkillsPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::LegendarySkillResetConfirmCallback::Run,
			0x20D);

		using LegendaryReset_t = void(RE::PlayerCharacter::PlayerSkills*, RE::ActorValue);
		static REL::Relocation<LegendaryReset_t> _LegendaryReset;

		auto& trampoline = SKSE::GetTrampoline();
		_LegendaryReset = trampoline.write_call<5>(
			hook.address(),
			+[](RE::PlayerCharacter::PlayerSkills* a_playerSkills, RE::ActorValue a_actorValue)
			{
				if (!CustomSkillsManager::IsOurMenuMode()) {
					_LegendaryReset(a_playerSkills, a_actorValue);
				}
			});
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

		struct Patch : Xbyak::CodeGenerator
		{
			Patch()
			{
				mov(rax, reinterpret_cast<std::uintptr_t>(IsLegendaryAvailable));
				jmp(rax);
			}
		};

		Patch patch{};
		patch.ready();

		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void Legendary::RefundPerksPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::RefundPerks, 0xED);
		REL::make_pattern<"44 00 B8 ?? ?? 00 00">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr)
			{
				mov(cl, r15b);
				mov(rax,
					reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::SetCurrentPerkPoints));
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
}
