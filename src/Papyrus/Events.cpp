#include "Events.h"

#include "CustomSkills/CustomSkillBookReadRegSet.h"
#include "CustomSkills/CustomSkillIncreaseRegSet.h"

#define REGISTER(vm, className, func) vm->RegisterFunction(#func##sv, className, &Events<T>::func)

namespace Papyrus
{
	using namespace ::CustomSkills;

	template <typename T>
	void Events<T>::RegisterForCustomSkillIncrease(RE::StaticFunctionTag*, T* akReceiver)
	{
		const auto reg = CustomSkillIncreaseRegSet::Get();
		reg->Register(akReceiver);
	}

	template <typename T>
	void Events<T>::UnregisterForCustomSkillIncrease(RE::StaticFunctionTag*, T* akReceiver)
	{
		const auto reg = CustomSkillIncreaseRegSet::Get();
		reg->Unregister(akReceiver);
	}

	template <typename T>
	void Events<T>::RegisterForCustomSkillBookRead(
		RE::StaticFunctionTag*,
		T* akReceiver,
		bool abReplaceDefault)
	{
		const auto reg = CustomSkillBookReadRegSet::Get();
		reg->Register(akReceiver, abReplaceDefault);
	}

	template <typename T>
	void Events<T>::UnregisterForCustomSkillBookRead(RE::StaticFunctionTag*, T* akReceiver)
	{
		const auto reg = CustomSkillBookReadRegSet::Get();
		reg->Unregister(akReceiver);
	}

	template <typename T>
	bool Events<T>::RegisterFuncs(RE::BSScript::IVirtualMachine* a_vm)
	{
		static constexpr auto className = []()
		{
			if constexpr (std::is_same_v<T, RE::TESForm>) {
				return "CustomSkills_FormExt"sv;
			}
			else if constexpr (std::is_same_v<T, RE::BGSBaseAlias>) {
				return "CustomSkills_AliasExt"sv;
			}
			else if constexpr (std::is_same_v<T, RE::ActiveEffect>) {
				return "CustomSkills_ActiveMagicEffectExt"sv;
			}
		}();

		REGISTER(a_vm, className, RegisterForCustomSkillIncrease);
		REGISTER(a_vm, className, UnregisterForCustomSkillIncrease);
		REGISTER(a_vm, className, RegisterForCustomSkillBookRead);
		REGISTER(a_vm, className, UnregisterForCustomSkillBookRead);

		return true;
	}

	template struct Events<RE::TESForm>;
	template struct Events<RE::BGSBaseAlias>;
	template struct Events<RE::ActiveEffect>;
}

#undef REGISTER_ALL
