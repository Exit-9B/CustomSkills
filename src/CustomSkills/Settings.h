#pragma once

#include "Skill.h"

namespace CustomSkills
{
	class Settings final
	{
	public:
		static auto ReadSkills() -> util::istring_map<std::shared_ptr<SkillGroup>>;

	private:
		inline static constexpr std::int32_t MaxNodes = 128;

		static auto ReadSkill(const std::filesystem::path& a_file) -> std::shared_ptr<SkillGroup>;

		static auto ReadLegacySkill(const std::filesystem::path& a_file)
			-> std::shared_ptr<SkillGroup>;

		static void Error(const std::filesystem::path& a_file, std::string_view a_message);
	};
}
