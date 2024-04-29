#pragma once

namespace CustomSkills
{
	class ActorValue final
	{
	public:
		static void WriteHooks();

	private:
		static float GetActorValue(
			const RE::ActorValueOwner* a_owner,
			RE::ActorValue a_actorValue);

		static float GetPermanentActorValue(
			const RE::ActorValueOwner* a_owner,
			RE::ActorValue a_actorValue);

		static float GetBaseActorValue(
			const RE::ActorValueOwner* a_owner,
			RE::ActorValue a_actorValue);

		static void SetBaseActorValue(
			RE::ActorValueOwner* a_owner,
			RE::ActorValue a_actorValue,
			float a_value);

		inline static REL::Relocation<decltype(&GetActorValue)> _GetActorValue;
		inline static REL::Relocation<decltype(&GetPermanentActorValue)> _GetPermanentActorValue;
		inline static REL::Relocation<decltype(&GetBaseActorValue)> _GetBaseActorValue;
		inline static REL::Relocation<decltype(&SetBaseActorValue)> _SetBaseActorValue;
	};
}
