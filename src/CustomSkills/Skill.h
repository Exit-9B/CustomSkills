#pragma once

namespace CustomSkills
{
	class Skill final
	{
	public:
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

		std::string Name;
		std::string Description;
		std::string Skydome;
		RE::TESGlobal* Level = nullptr;
		RE::TESGlobal* Ratio = nullptr;
		RE::TESGlobal* ShowLevelup = nullptr;
		RE::TESGlobal* OpenMenu = nullptr;
		RE::TESGlobal* PerkPoints = nullptr;
		RE::TESGlobal* Legendary = nullptr;
		RE::TESGlobal* Color = nullptr;
		RE::TESGlobal* DebugReload = nullptr;
		bool NormalNif = false;
		RE::BGSSkillPerkTreeNode* SkillTree = nullptr;
		std::string ColorStr;
		std::int32_t ColorLast = -1;
	};
}
