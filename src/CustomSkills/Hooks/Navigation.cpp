#include "Navigation.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

#include <xbyak/xbyak.h>

namespace CustomSkills
{
	void Navigation::WriteHooks()
	{
		RotatePatch();
		EffectShaderPatch();
	}

	void Navigation::RotatePatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(RE::Offset::StatsMenu::Rotate, 0x46);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::IsSingleSkillMode);
	}

	void Navigation::EffectShaderPatch()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::StatsMenu::ProcessRotateEvent,
			0x15C);
		REL::make_pattern<"80 3D ?? ?? ?? ?? 00">().match_or_fail(hook.address());

		util::write_disp(
			hook.address() + 0x2,
			hook.address() + 0x7,
			CustomSkillsManager::IsSingleSkillMode);
	}
}
