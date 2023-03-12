#pragma once

#include "Skill.h"

namespace CustomSkills
{
	class Settings final
	{
	public:
		static auto ReadSkills() -> std::map<std::string, std::shared_ptr<Skill>>;

	private:
		inline static constexpr std::int32_t MaxNodes = 128;

		static auto ReadSkill(const std::filesystem::path& a_file) -> std::shared_ptr<Skill>;

		static void Error(const std::filesystem::path& a_file, std::string_view a_message);
	};
}
