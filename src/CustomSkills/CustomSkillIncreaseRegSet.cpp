#include "CustomSkillIncreaseRegSet.h"

namespace CustomSkills
{
	CustomSkillIncreaseRegSet* CustomSkillIncreaseRegSet::Get()
	{
		static CustomSkillIncreaseRegSet instance{};
		return &instance;
	}

	void CustomSkillIncreaseRegSet::ObjectDeleted(RE::VMHandle a_handle)
	{
		const auto it = _handles.find(a_handle);
		if (it != _handles.end()) {
			RE::VMHandle handle = *it;
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
			if (policy) {
				policy->ReleaseHandle(handle);
			}
			_handles.erase(it);
		}
	}
}
