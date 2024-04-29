#pragma once

namespace CustomSkills
{
	class CustomSkillBookReadRegSet
	{
	public:
		static constexpr auto EventName = "OnCustomSkillBookRead"sv;

		static CustomSkillBookReadRegSet* Get();

		bool Register(const RE::TESForm* a_form, bool a_replaceDefault);
		bool Register(const RE::BGSBaseAlias* a_alias, bool a_replaceDefault);
		bool Register(const RE::ActiveEffect* a_effect, bool a_replaceDefault);
		bool Unregister(const RE::TESForm* a_form);
		bool Unregister(const RE::BGSBaseAlias* a_alias);
		bool Unregister(const RE::ActiveEffect* a_effect);
		void Clear();
		bool Save(
			SKSE::SerializationInterface* a_intfc,
			std::uint32_t a_type,
			std::uint32_t a_version);
		bool Save(SKSE::SerializationInterface* a_intfc);
		bool Load(SKSE::SerializationInterface* a_intfc);
		void SendEvent(std::string_view a_skillId, std::int32_t a_increment);

		void ObjectDeleted(RE::VMHandle a_handle);

	private:
		using Lock = std::recursive_mutex;
		using Locker = std::lock_guard<Lock>;

		bool Register(const void* a_object, RE::VMTypeID a_typeID, bool a_replaceDefault);

		bool Unregister(const void* a_object, RE::VMTypeID a_typeID);

		template <class Tuple, std::size_t... I>
		inline void SendEvent_Tuple(Tuple&& a_tuple, std::index_sequence<I...>)
		{
			SendEvent(std::get<I>(std::forward<Tuple>(a_tuple)).Unpack()...);
		}

		std::map<RE::VMHandle, bool> _handles;
		mutable Lock _lock;
	};
}
