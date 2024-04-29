#include "Training.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Training::WriteHooks()
	{
		MenuSkillPatch();
		SkillNamePatch();
		MaxLevelPatch();
		IncrementSkillPatch();
	}

	void Training::MenuSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::TrainingMenu::SetTrainer, 0x63);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using GetTeachesSkill_t = RE::ActorValue(RE::TESClass::*)() const;
		static REL::Relocation<GetTeachesSkill_t> _GetTeachesSkill;

		auto GetTeachesSkill = +[](const RE::TESClass* a_class) -> RE::ActorValue
		{
			if (CustomSkillsManager::IsOurTrainingMode()) {
				return static_cast<RE::ActorValue>(CUSTOM_SKILL_BASE_VALUE);
			}
			else {
				return _GetTeachesSkill(a_class);
			}
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_GetTeachesSkill = trampoline.write_call<5>(hook.address(), GetTeachesSkill);
	}

	void Training::SkillNamePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::GetActorValueName, 0x83);
		REL::make_pattern<"33 C0 48 83 C4 20">().match_or_fail(hook.address());

		auto GetSkillName = +[]() -> const char*
		{
			if (CustomSkillsManager::IsOurTrainingMode()) {
				return CustomSkillsManager::_trainingSkill->GetName().data();
			}
			return nullptr;
		};

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr, std::uintptr_t a_retnAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label retnLbl;

				call(ptr[rip + funcLbl]);
				add(rsp, 0x20);
				jmp(ptr[rip + retnLbl]);

				L(funcLbl);
				dq(a_funcAddr);

				L(retnLbl);
				dq(a_retnAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(GetSkillName),
			hook.address() + 0x6);
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}

	void Training::MaxLevelPatch()
	{
		auto GetMaximumTrainingLevel = +[](const RE::TESClass* a_class) -> std::uint32_t
		{
			if (CustomSkillsManager::IsOurTrainingMode()) {
				return CustomSkillsManager::_trainingMax;
			}
			return a_class->data.maximumTrainingLevel;
		};

		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::TESClass::GetMaximumTrainingLevel);
		REL::safe_fill(hook.address(), 0x10, REL::INT3);
		util::write_14branch(hook.address(), GetMaximumTrainingLevel);
	}

	void Training::IncrementSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::TrainingMenu::Train, 0xD8);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		using IncrementSkill_t = void (RE::PlayerCharacter::*)(RE::ActorValue, std::uint32_t);
		static REL::Relocation<IncrementSkill_t> _IncrementSkill;

		auto IncrementSkill =
			+[](RE::PlayerCharacter* a_player, RE::ActorValue a_actorValue, std::uint32_t a_count)
		{
			if (CustomSkillsManager::IsOurTrainingMode()) {
				const auto& skill = CustomSkillsManager::_trainingSkill;
				skill->Increment(a_count);
			}
			else {
				return _IncrementSkill(a_player, a_actorValue, a_count);
			}
		};

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_IncrementSkill = trampoline.write_call<5>(hook.address(), IncrementSkill);
	}
}
