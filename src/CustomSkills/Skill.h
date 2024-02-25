#pragma once

namespace CustomSkills
{
	class Skill final
	{
	public:
		std::string_view GetName() const
		{
			return Info ? std::string_view(Info->fullName) : ""sv;
		}

		float GetLevel() const { return Level ? Level->value : 0.0f; }

		bool UpdateColor()
		{
			if (!Color) {
				return false;
			}

			auto cv = static_cast<std::int32_t>(Color->value) & 0xFFFFFF;
			if (cv == ColorLast) {
				return true;
			}

			ColorLast = cv;
			ColorStr = fmt::format("#{:06X}", cv);
			return true;
		}

		void LegendaryReset(float a_resetLevel)
		{
			if (Level) {
				Level->value = a_resetLevel;
			}

			if (Ratio) {
				Ratio->value = 0.0f;
			}

			if (Legendary) {
				Legendary->value++;
			}
		}

		std::string ID;
		std::string Description;
		RE::TESGlobal* Level = nullptr;
		RE::TESGlobal* Ratio = nullptr;
		RE::TESGlobal* ShowLevelup = nullptr;
		RE::TESGlobal* Legendary = nullptr;
		RE::TESGlobal* Color = nullptr;
		RE::ActorValueInfo* Info = nullptr;
		std::uint32_t SkillTreeWidth = 3;
		std::string ColorStr;
		std::int32_t ColorLast = -1;
	};

	class SkillGroup final
	{
	public:
		std::string Skydome;
		std::uint32_t CameraRightPoint = 2;
		std::uint32_t LastSelectedTree = 0;
		RE::TESGlobal* OpenMenu = nullptr;
		RE::TESGlobal* PerkPoints = nullptr;
		RE::TESGlobal* DebugReload = nullptr;
		std::vector<std::shared_ptr<Skill>> Skills;
	};
}
