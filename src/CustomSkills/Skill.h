#pragma once

#include "CustomSkills/Game.h"

namespace CustomSkills
{
	class Skill final
	{
	public:
		std::string_view GetName() const { return Info ? std::string_view(Info->fullName) : ""sv; }

		float GetLevel() const { return Level ? Level->value : 0.0f; }

		float GetProgressPercent() const { return Ratio ? Ratio->value * 100.0f : 0.0f; }

		static constexpr std::int32_t MaxLevel() { return 100; }

		void Advance(float a_magnitude, bool a_isSkillUse = true, bool a_hideNotification = false);

		void Increment(std::uint32_t a_count);

		bool UpdateColor();

		std::string ID;
		std::string Description;
		RE::TESGlobal* Level = nullptr;
		RE::TESGlobal* Ratio = nullptr;
		RE::TESGlobal* ShowLevelup = nullptr;
		RE::TESGlobal* Legendary = nullptr;
		RE::TESGlobal* Color = nullptr;
		RE::ActorValueInfo* Info = nullptr;
		RE::TESForm* AdvanceObject = nullptr;
		std::string ColorStr;
		std::int32_t ColorLast = -1;
		bool EnableXPPerRank = false;
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
		std::vector<RE::ActorValue> ActorValues;
	};
}
