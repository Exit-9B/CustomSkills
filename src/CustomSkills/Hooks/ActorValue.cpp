#include "ActorValue.h"

#include "CustomSkills/CustomSkillsManager.h"
#include "RE/Offset.h"

namespace CustomSkills
{
	void ActorValue::WriteHooks()
	{
		ActorValueOwnerPatch();
	}

	void ActorValue::ActorValueOwnerPatch()
	{
		auto vtbl = REL::Relocation<std::uintptr_t>(
			RE::Offset::PlayerCharacter::Vtbl_ActorValueOwner);

		_GetActorValue = vtbl.write_vfunc(1, &ActorValue::GetActorValue);
		_GetPermanentActorValue = vtbl.write_vfunc(2, &ActorValue::GetActorValue);
		_GetBaseActorValue = vtbl.write_vfunc(3, &ActorValue::GetBaseActorValue);
		_SetBaseActorValue = vtbl.write_vfunc(4, &ActorValue::SetBaseActorValue);
	}

	float ActorValue::GetActorValue(
		const RE::ActorValueOwner* a_owner,
		RE::ActorValue a_actorValue)
	{
		if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
			return skill->GetLevel();
		}

		return _GetActorValue(a_owner, a_actorValue);
	}

	float ActorValue::GetPermanentActorValue(
		const RE::ActorValueOwner* a_owner,
		RE::ActorValue a_actorValue)
	{
		if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
			return skill->GetLevel();
		}

		return _GetPermanentActorValue(a_owner, a_actorValue);
	}

	float ActorValue::GetBaseActorValue(
		const RE::ActorValueOwner* a_owner,
		RE::ActorValue a_actorValue)
	{
		if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
			return skill->GetLevel();
		}

		return _GetBaseActorValue(a_owner, a_actorValue);
	}

	void ActorValue::SetBaseActorValue(
		RE::ActorValueOwner* a_owner,
		RE::ActorValue a_actorValue,
		float a_value)
	{
		if (const auto skill = CustomSkillsManager::GetCurrentSkill(a_actorValue)) {
			if (skill->Level) {
				skill->Level->value = a_value;
			}
		}
		else {
			_SetBaseActorValue(a_owner, a_actorValue, a_value);
		}
	}
}
