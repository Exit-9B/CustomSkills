#pragma once

namespace Papyrus
{
	template <typename T>
	struct Events
	{
		static void RegisterForCustomSkillIncrease(RE::StaticFunctionTag*, T* akReceiver);

		static void UnregisterForCustomSkillIncrease(RE::StaticFunctionTag*, T* akReceiver);

		static void RegisterForCustomSkillBookRead(
			RE::StaticFunctionTag*,
			T* akReceiver,
			bool abReplaceDefault);

		static void UnregisterForCustomSkillBookRead(RE::StaticFunctionTag*, T* akReceiver);

		static bool RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm);
	};

	extern template struct Events<RE::TESForm>;
	extern template struct Events<RE::BGSBaseAlias>;
	extern template struct Events<RE::ActiveEffect>;
}
