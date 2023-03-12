#pragma once

namespace Papyrus
{
	namespace CustomSkills
	{
		std::int32_t GetAPIVersion(RE::StaticFunctionTag*);

		void OpenCustomSkillMenu(RE::StaticFunctionTag*, RE::BSFixedString asSkillId);

		void ShowSkillIncreaseMessage(
			RE::StaticFunctionTag*,
			RE::BSFixedString asSkillId,
			std::int32_t aiSkillLevel);

		bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm);
	}
}
