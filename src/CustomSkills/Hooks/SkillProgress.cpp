#include "SkillProgress.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Game.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void SkillProgress::WriteHooks()
	{
		CurrentPerkPointsPatch();
		SelectPerkPatch();
		SkillProgressPatch();
		//SkillLevelPatch();
		SkillLevelPatch2();
		PerkViewSkillLevelPatch();
		RequirementsTextPatch();
	}

	void SkillProgress::CurrentPerkPointsPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::GetPerkCount, 0xE1);
		REL::make_pattern<"0F B6 80">().match_or_fail(hook.address());

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x7);
		trampoline.write_call<6>(hook.address(), &CustomSkillsManager::GetCurrentPerkPoints);
	}

	void SkillProgress::SelectPerkPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SelectPerk, 0x1CD);
		REL::make_pattern<"E8">().match_or_fail(hook.address());

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		_ModifyPerkCount = trampoline.write_call<5>(hook.address(), &ModifyPerkCount);
	}

	void SkillProgress::SkillProgressPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::PlayerSkills::GetSkillProgress);

		REL::safe_fill(hook.address(), REL::INT3, 0x50);
		util::write_14branch(hook.address(), &GetSkillProgress);
	}

	void SkillProgress::SkillLevelPatch2()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x16DD);

		REL::make_pattern<
			"48 8B 0D ?? ?? ?? ?? "
			"48 81 C1 ?? 00 00 00 "
			"48 8B 01 "
			"41 8B D7 "
			"FF 50 08">()
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

		Patch patch{ reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetSkillLevel) };
		patch.ready();

		assert(patch.getSize() <= 0x17);

		REL::safe_fill(hook.address(), REL::NOP, 0x17);
		REL::safe_write(hook.address(), patch.getCode(), patch.getSize());
	}

	void SkillProgress::PerkViewSkillLevelPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::SetSkillInfo, 0x118F);

		REL::make_pattern<"8B 95 78 06 00 00 FF 50 18">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_funcAddr)
			{
				Xbyak::Label funcLbl;

				mov(ecx, ptr[rbp + 0x678]);
				jmp(ptr[rip + funcLbl]);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(
			reinterpret_cast<std::uintptr_t>(&CustomSkillsManager::GetBaseSkillLevel));
		patch->ready();

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		REL::safe_fill(hook.address(), REL::NOP, 0x9);
		trampoline.write_call<6>(hook.address(), patch->getCode());
	}

	void SkillProgress::RequirementsTextPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::BGSPerk::GetRequirementsText);

		REL::safe_fill(hook.address(), REL::INT3, 0x100);
		util::write_14branch(hook.address(), &GetRequirementsText);
	}

	void SkillProgress::ModifyPerkCount(RE::StatsMenu* a_statsMenu, std::int32_t a_countDelta)
	{
		if (CustomSkillsManager::IsOurMenuMode()) {
			if (a_countDelta > 0) {
				if (const auto player = RE::PlayerCharacter::GetSingleton()) {
					std::int32_t oldCount = player->perkCount;
					std::int32_t newCount = oldCount + a_countDelta;
					if (newCount > oldCount && oldCount != 255) {
						player->perkCount = static_cast<std::uint8_t>((std::min)(255, newCount));
					}
				}
			}
			else {
				std::int32_t oldCount = CustomSkillsManager::GetCurrentPerkPoints();
				std::int32_t newCount = oldCount + a_countDelta;

				if (newCount < 0) {
					newCount = 0;
				}
				else if (newCount > 255) {
					newCount = 255;
				}
				CustomSkillsManager::SetCurrentPerkPoints(static_cast<std::uint8_t>(newCount));
			}
		}
		else {
			_ModifyPerkCount(a_statsMenu, a_countDelta);
		}
	}

	void SkillProgress::GetSkillProgress(
		RE::PlayerCharacter::PlayerSkills* a_playerSkills,
		RE::ActorValue a_skill,
		float* a_level,
		float* a_xp,
		float* a_levelThreshold,
		std::uint32_t* a_legendary)
	{
		if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_skill)) {
			if (skill->Level) {
				*a_level = skill->Level->value;
			}
			else {
				*a_level = 1.0f;
			}

			if (skill->Ratio) {
				*a_xp = skill->Ratio->value;
			}
			else {
				*a_xp = 0.0f;
			}

			*a_levelThreshold = 1.0f;

			if (a_legendary) {
				if (skill->Legendary) {
					*a_legendary = static_cast<std::uint32_t>(skill->Legendary->value);
				}
				else {
					*a_legendary = 0;
				}
			}
		}
		else {
			std::size_t idx = util::to_underlying(a_skill) - 6;
			auto& data = a_playerSkills->data->skills[idx];
			*a_level = data.level;
			*a_xp = data.xp;
			*a_levelThreshold = data.levelThreshold;
			if (a_legendary) {
				*a_legendary = a_playerSkills->data->legendaryLevels[idx];
			}
		}
	}

	void SkillProgress::GetRequirementsText(
		RE::BGSPerk* a_perk,
		char* a_buf,
		std::int32_t a_bufLen,
		const char* a_prefix,
		const char* a_suffix)
	{
		*a_buf = '\0';

		std::ostringstream ss;
		bool firstReq = true;

		for (const auto* item = a_perk->perkConditions.head; item; item = item->next) {
			std::string token;

			auto& functionData = item->data.functionData;
			switch (functionData.function.get()) {
			case RE::FUNCTION_DATA::FunctionID::kGetBaseActorValue:
			{
				auto param = static_cast<RE::ActorValue>(
					std::bit_cast<std::uint64_t>(functionData.params[0]));
				auto name = Game::GetActorValueName(param);
				if (!name) {
					break;
				}

				token = fmt::format(
					"{}{}{}",
					a_prefix,
					static_cast<std::int32_t>(item->data.comparisonValue.f),
					a_suffix);
			} break;
			case RE::FUNCTION_DATA::FunctionID::kGetGlobalValue:
			{
				auto param = std::bit_cast<RE::TESGlobal*>(functionData.params[0]);
				if (!param) {
					break;
				}

				auto sk = CustomSkillsManager::FindSkillFromGlobalLevel(param);
				if (!sk) {
					break;
				}

				token = fmt::format(
					"{}{}{}",
					a_prefix,
					static_cast<std::int32_t>(item->data.comparisonValue.f),
					a_suffix);
			} break;
			}

			if (token.empty()) {
				continue;
			}

			static auto sRequirementsText = RE::GameSettingCollection::GetSingleton()->GetSetting(
				"sRequirementsText");
			if (firstReq) {
				ss << sRequirementsText->GetString();
			}
			else {
				ss << ",";
				firstReq = false;
			}

			ss << " ";

			std::size_t pos = ss.tellp();
			auto newSize = pos + token.size();
			if (newSize + 6 < a_bufLen) {
				ss << token;
			}
			else {
				ss << "...";
				break;
			}
		}

		auto str = ss.str();
		assert(str.size() < a_bufLen);
		std::copy_n(str.data(), str.size() + 1, a_buf);
	}
}
