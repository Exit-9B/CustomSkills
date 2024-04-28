#include "CustomSkillBookReadRegSet.h"

#include "CustomSkillsManager.h"

namespace CustomSkills
{
	CustomSkillBookReadRegSet* CustomSkillBookReadRegSet::Get()
	{
		static CustomSkillBookReadRegSet instance{};
		return &instance;
	}

	bool CustomSkillBookReadRegSet::Register(const RE::TESForm* a_form, bool a_replaceDefault)
	{
		assert(a_form);
		return Register(
			a_form,
			static_cast<RE::VMTypeID>(a_form->GetFormType()),
			a_replaceDefault);
	}

	bool CustomSkillBookReadRegSet::Register(
		const RE::BGSBaseAlias* a_alias,
		bool a_replaceDefault)
	{
		assert(a_alias);
		return Register(a_alias, a_alias->GetVMTypeID(), a_replaceDefault);
	}

	bool CustomSkillBookReadRegSet::Register(
		const RE::ActiveEffect* a_effect,
		bool a_replaceDefault)
	{
		assert(a_effect);
		return Register(a_effect, RE::ActiveEffect::VMTYPEID, a_replaceDefault);
	}

	bool CustomSkillBookReadRegSet::Unregister(const RE::TESForm* a_form)
	{
		assert(a_form);
		return Unregister(a_form, static_cast<RE::VMTypeID>(a_form->GetFormType()));
	}

	bool CustomSkillBookReadRegSet::Unregister(const RE::BGSBaseAlias* a_alias)
	{
		assert(a_alias);
		return Unregister(a_alias, a_alias->GetVMTypeID());
	}

	bool CustomSkillBookReadRegSet::Unregister(const RE::ActiveEffect* a_effect)
	{
		assert(a_effect);
		return Unregister(a_effect, RE::ActiveEffect::VMTYPEID);
	}

	void CustomSkillBookReadRegSet::Clear()
	{
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		Locker locker(_lock);
		if (policy) {
			for (auto& [handle, replaceDefault] : _handles) {
				policy->ReleaseHandle(handle);
			}
		}
		_handles.clear();
	}

	bool CustomSkillBookReadRegSet::Save(
		SKSE::SerializationInterface* a_intfc,
		std::uint32_t a_type,
		std::uint32_t a_version)
	{
		assert(a_intfc);
		if (!a_intfc->OpenRecord(a_type, a_version)) {
			logger::error("Failed to open record");
			return false;
		}

		return Save(a_intfc);
	}

	bool CustomSkillBookReadRegSet::Save(SKSE::SerializationInterface* a_intfc)
	{
		assert(a_intfc);
		Locker locker(_lock);
		const std::size_t numRegs = _handles.size();
		if (!a_intfc->WriteRecordData(numRegs)) {
			logger::error("Failed to save number of regs ({})", numRegs);
			return false;
		}

		for (auto& [handle, replaceDefault] : _handles) {
			if (!a_intfc->WriteRecordData(handle)) {
				logger::error("Failed to save reg ({})", handle);
				return false;
			}

			if (!a_intfc->WriteRecordData(replaceDefault)) {
				logger::error("Failed to save replaceDefault flag ({})", handle);
				return false;
			}
		}

		return true;
	}

	bool CustomSkillBookReadRegSet::Load(SKSE::SerializationInterface* a_intfc)
	{
		assert(a_intfc);
		std::size_t numRegs;
		a_intfc->ReadRecordData(numRegs);

		Locker locker(_lock);
		_handles.clear();
		RE::VMHandle handle;
		std::uint8_t replaceDefault;
		for (std::size_t i = 0; i < numRegs; ++i) {
			a_intfc->ReadRecordData(handle);
			a_intfc->ReadRecordData(replaceDefault);
			if (!a_intfc->ResolveHandle(handle, handle)) {
				logger::warn("Failed to resolve handle ({})", handle);
				continue;
			}

			auto result = _handles.emplace(handle, replaceDefault);
			if (!result.second) {
				logger::error("Loaded duplicate handle ({})", handle);
			}
		}

		return true;
	}

	bool CustomSkillBookReadRegSet::Register(
		const void* a_object,
		RE::VMTypeID a_typeID,
		bool a_replaceDefault)
	{
		assert(a_object);
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!policy) {
			logger::error("Failed to get handle policy");
			return false;
		}

		const auto invalidHandle = policy->EmptyHandle();
		const auto handle = policy->GetHandleForObject(a_typeID, a_object);
		if (handle == invalidHandle) {
			logger::error("Failed to create handle");
			return false;
		}

		_lock.lock();
		auto result = _handles.emplace(handle, a_replaceDefault);
		_lock.unlock();

		if (!result.second) {
			logger::warn("Handle already registered ({})", handle);
		}
		else {
			policy->PersistHandle(handle);
		}
		return result.second;
	}

	bool CustomSkillBookReadRegSet::Unregister(const void* a_object, RE::VMTypeID a_typeID)
	{
		assert(a_object);
		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
		if (!policy) {
			logger::error("Failed to get handle policy!");
			return false;
		}

		auto invalidHandle = policy->EmptyHandle();
		auto handle = policy->GetHandleForObject(a_typeID, a_object);
		if (handle == invalidHandle) {
			logger::error("Failed to create handle!");
			return false;
		}

		Locker locker(_lock);
		auto it = _handles.find(handle);
		if (it == _handles.end()) {
			logger::warn("Could not find registration");
			return false;
		}
		else {
			policy->ReleaseHandle(it->first);
			_handles.erase(it);
			return true;
		}
	}

	void CustomSkillBookReadRegSet::SendEvent(std::string_view a_skillId, std::int32_t a_increment)
	{
		RE::BSFixedString eventName(EventName);

		auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
		bool runDefault = true;
		for (auto& [handle, replaceDefault] : _handles) {
			auto args = RE::MakeFunctionArguments(
				RE::BSFixedString(a_skillId),
				std::int32_t(a_increment));
			vm->SendEvent(handle, eventName, args);
			runDefault &= !replaceDefault;
		}

		if (runDefault) {
			if (const auto skill = CustomSkillsManager::FindSkill(a_skillId)) {
				skill->Increment(a_increment);
			}
		}
	}

	void CustomSkillBookReadRegSet::ObjectDeleted(RE::VMHandle a_handle)
	{
		const auto it = _handles.find(a_handle);
		if (it != _handles.end()) {
			RE::VMHandle handle = it->first;
			auto vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
			auto policy = vm ? vm->GetObjectHandlePolicy() : nullptr;
			if (policy) {
				policy->ReleaseHandle(handle);
			}
			_handles.erase(it);
		}
	}
}
