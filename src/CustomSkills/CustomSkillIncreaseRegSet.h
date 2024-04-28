#pragma once

namespace CustomSkills
{
	class CustomSkillIncreaseRegSet : public SKSE::RegistrationSet<std::string>
	{
	public:
		using super = SKSE::RegistrationSet<std::string>;

		static CustomSkillIncreaseRegSet* Get();

		CustomSkillIncreaseRegSet() : super("OnCustomSkillIncrease"sv) {}
	};
}
