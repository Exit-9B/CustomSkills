#pragma once

namespace CustomSkills::Serialization
{
	constexpr std::uint32_t Version = 1;
	constexpr std::uint32_t ID = 'CSFW';
	constexpr std::uint32_t SkillIncreaseEvent = 'CSLI';
	constexpr std::uint32_t SkillBookReadEvent = 'CSBR';

	void FormDeleteCallback(RE::VMHandle a_handle);

	void LoadCallback(SKSE::SerializationInterface* a_intfc);

	void RevertCallback(SKSE::SerializationInterface* a_intfc);

	void SaveCallback(SKSE::SerializationInterface* a_intfc);
}
