#include "SkillBooks.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void SkillBooks::WriteHooks()
	{
		CustomSkillPatch();
	}

	static bool ReadSkillBook(RE::TESObjectBOOK* a_book)
	{
		if (a_book->IsRead())
			return false;

		for (const RE::BGSKeyword* const keyword :
			 std::span(a_book->keywords, a_book->numKeywords)) {

			if (!keyword)
				continue;

			const auto& str = keyword->formEditorID;
			constexpr auto prefix = "CustomSkillBook_"sv;
			constexpr auto pos = prefix.size();
			if (str.size() <= pos || ::_strnicmp(str.data(), prefix.data(), pos) != 0) {
				continue;
			}

			const auto skill = CustomSkillsManager::FindSkill(str.data() + pos);
			if (!skill)
				continue;

			const auto player = RE::PlayerCharacter::GetSingleton();
			float count = 1.0f;
			RE::BGSEntryPoint::HandleEntryPoint(
				RE::BGSEntryPoint::ENTRY_POINT::kAdjustBookSkillPoints,
				player,
				&count);

			skill->Increment(static_cast<std::int32_t>(count));
			return true;
		}

		return false;
	}

	void SkillBooks::CustomSkillPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::TESObjectBOOK::Read, 0x103);
		REL::make_pattern<"41 BD FF FF FF FF">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
			{
				Xbyak::Label funcLbl;
				Xbyak::Label learnedSkill;
				Xbyak::Label noSkill;

				mov(r13d, 0xFFFFFFFF);
				mov(rcx, r15);
				call(ptr[rip + funcLbl]);
				cmp(al, 0);
				jnz(learnedSkill);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x6);

				L(learnedSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x70);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(hook.address(), reinterpret_cast<std::uintptr_t>(&ReadSkillBook));
		patch->ready();

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<6>(hook.address(), patch->getCode());
	}
}
