#include "Crafting.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Crafting::WriteHooks()
	{
		ConstructibleObjectBottomBarPatch();
		ConstructibleObjectCreationPatch();
	}

	[[nodiscard]] static std::shared_ptr<Skill> GetWorkbenchSkill(
		const RE::TESFurniture* a_furniture)
	{
		for (const RE::BGSKeyword* const keyword :
			 std::span(a_furniture->keywords, a_furniture->numKeywords)) {

			if (!keyword)
				continue;

			const auto str = std::string_view(keyword->formEditorID);
			static constexpr auto prefix = "CustomSkillWorkbench_"sv;
			if (str.size() <= prefix.size() ||
				::_strnicmp(str.data(), prefix.data(), prefix.size()) != 0) {
				continue;
			}

			const auto skill = CustomSkillsManager::FindSkill(
				std::string(str.substr(prefix.size())));

			if (skill) {
				return skill;
			}
		}

		return nullptr;
	}

	static bool UpdateBottomBar(RE::CraftingSubMenus::ConstructibleObjectMenu* a_menu)
	{
		const auto skill = GetWorkbenchSkill(a_menu->furniture);
		if (!skill)
			return false;

		std::array<RE::GFxValue, 3> craftingInfo;
		craftingInfo[0].SetString(skill->GetName());
		craftingInfo[1].SetNumber(skill->GetLevel());
		craftingInfo[2].SetNumber(skill->GetProgressPercent());

		a_menu->bottomBarInfo.Invoke("UpdateCraftingInfo", craftingInfo);

		return true;
	}

	void Crafting::ConstructibleObjectBottomBarPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::CraftingSubMenus::ConstructibleObjectMenu::UpdateBottomBar,
			0x37D);
		REL::make_pattern<"80 F9 11 77 0D">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
			{
				Xbyak::Label customSkill;
				Xbyak::Label noSkill;
				Xbyak::Label funcLbl;

				cmp(cl, 0x11);
				ja(customSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(customSkill);
				mov(rcx, rdi);
				call(ptr[rip + funcLbl]);
				cmp(al, 0);
				jz(noSkill);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0xD);

				L(noSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x12);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(hook.address(), reinterpret_cast<std::uintptr_t>(&UpdateBottomBar));
		patch->ready();

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}

	static void UseWorkbench(const RE::TESFurniture* a_furniture, float a_amount)
	{
		if (const auto skill = GetWorkbenchSkill(a_furniture)) {
			skill->Advance(a_amount);
		}
	}

	void Crafting::ConstructibleObjectCreationPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::CraftingSubMenus::ConstructibleObjectMenu::CreationConfirmed,
			0x83);
		REL::make_pattern<"83 F8 11 77 1E">().match_or_fail(hook.address());

		struct Patch : Xbyak::CodeGenerator
		{
			Patch(std::uintptr_t a_hookAddr, std::uintptr_t a_funcAddr)
			{
				Xbyak::Label customSkill;
				Xbyak::Label funcLbl;

				cmp(eax, 0x11);
				ja(customSkill);
				jmp(ptr[rip]);
				dq(a_hookAddr + 0x5);

				L(customSkill);
				mov(rcx,
					ptr[r15 + offsetof(RE::CraftingSubMenus::ConstructibleObjectMenu, furniture)]);
				movaps(xmm1, xmm0);
				call(ptr[rip + funcLbl]);

				jmp(ptr[rip]);
				dq(a_hookAddr + 0x23);

				L(funcLbl);
				dq(a_funcAddr);
			}
		};

		auto patch = new Patch(hook.address(), reinterpret_cast<std::uintptr_t>(&UseWorkbench));
		patch->ready();

		// TRAMPOLINE: 14
		auto& trampoline = SKSE::GetTrampoline();
		trampoline.write_branch<5>(hook.address(), patch->getCode());
	}
}
