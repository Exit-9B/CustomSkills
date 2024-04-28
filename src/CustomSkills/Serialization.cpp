#include "Serialization.h"

#include "CustomSkillBookReadRegSet.h"
#include "CustomSkillIncreaseRegSet.h"

namespace CustomSkills::Serialization
{
	[[nodiscard]] static std::string TypeCodeToString(std::uint32_t a_typeCode)
	{
		std::ostringstream ss;
		for (std::size_t i = 0; i < sizeof(a_typeCode); ++i) {
			const char c = (a_typeCode >> (8 * i)) & 0xFF;
			if (std::isprint(c))
				ss << c;
			else
				ss << "\\x" << std::setfill('0') << std::setw(4) << static_cast<int>(c);
		}
		return ss.str();
	}

	void FormDeleteCallback(RE::VMHandle a_handle)
	{
		CustomSkillIncreaseRegSet::Get()->ObjectDeleted(a_handle);
		CustomSkillBookReadRegSet::Get()->ObjectDeleted(a_handle);
	}

	void LoadCallback(SKSE::SerializationInterface* a_intfc)
	{
		std::uint32_t type;
		std::uint32_t version;
		std::uint32_t length;
		while (a_intfc->GetNextRecordInfo(type, version, length)) {
			if (version > Version) {
				logger::critical("Loaded data is incompatible with plugin version"sv);
			}

			switch (type) {
			case SkillIncreaseEvent:
			{
				auto regs = CustomSkillIncreaseRegSet::Get();
				regs->Clear();
				if (!regs->Load(a_intfc)) {
					logger::critical("Failed to load CustomSkillIncrease regs"sv);
				}
			} break;
			case SkillBookReadEvent:
			{
				auto regs = CustomSkillBookReadRegSet::Get();
				regs->Clear();
				if (!regs->Load(a_intfc)) {
					logger::critical("Failed to load CustomSkillBookRead regs"sv);
				}
			} break;
			default:
			{
				logger::critical("Unrecognized record type ({})"sv, TypeCodeToString(type));
			} break;
			}
		}
	}

	void RevertCallback([[maybe_unused]] SKSE::SerializationInterface* a_intfc)
	{
		CustomSkillIncreaseRegSet::Get()->Clear();
		CustomSkillBookReadRegSet::Get()->Clear();
	}

	void SaveCallback(SKSE::SerializationInterface* a_intfc)
	{
		if (!CustomSkillIncreaseRegSet::Get()->Save(a_intfc, SkillIncreaseEvent, Version)) {
			logger::critical("Failed to save CustomSkillIncrease regs"sv);
		}
		if (!CustomSkillBookReadRegSet::Get()->Save(a_intfc, SkillBookReadEvent, Version)) {
			logger::critical("Failed to save CustomSkillBookRead regs"sv);
		}
	}
}
