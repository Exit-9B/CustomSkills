#include "Skill.h"

#include "CustomSkillIncreaseRegSet.h"
#include "EventSources.h"

namespace CustomSkills
{
	static float CalcLevelThreshold(
		std::int32_t a_currentRank,
		float a_improveMult,
		float a_improveOffset)
	{
		return std::fma(
			std::powf(static_cast<float>(a_currentRank), "fSkillUseCurve"_gs.value_or(1.95f)),
			a_improveMult,
			a_improveOffset);
	}

	static void IncreasePlayerCharacterXP(std::int32_t a_rankGained)
	{
		const auto player = RE::PlayerCharacter::GetSingleton();
		player->skills->data->xp += a_rankGained * "fXPPerSkillRank"_gs.value_or(1.0f);
	}

	void Skill::Advance(
		float a_magnitude,
		RE::TESForm* a_advanceObject,
		bool a_isSkillUse,
		bool a_hideNotification)
	{
		if (!Level || !Ratio || !Info || !Info->skill)
			return;

		const float useMult = Info->skill->useMult;
		if (useMult == 0.0f)
			return;
		const float useOffset = Info->skill->useOffset;
		const float improveMult = Info->skill->improveMult;
		const float improveOffset = Info->skill->improveOffset;

		float xp = a_isSkillUse ? std::fma(a_magnitude, useMult, useOffset) : a_magnitude;
		const auto player = RE::PlayerCharacter::GetSingleton();
		player->advanceSkill = RE::ActorValue::kNone;
		player->advanceObject = a_advanceObject;
		player->advanceAction = 0;
		RE::BGSEntryPoint::HandleEntryPoint(
			RE::BGSEntryPoint::ENTRY_POINT::kModSkillUse,
			player,
			&xp);
		player->advanceObject = nullptr;

		std::int32_t level = static_cast<std::int32_t>(Level->value);
		float ratio = Ratio->value;

		while (level < MaxLevel() && xp > 0.0f) {
			const float levelThreshold = CalcLevelThreshold(level, improveMult, improveOffset);

			if (ratio + xp / levelThreshold >= 1.0f - std::numeric_limits<float>::epsilon()) {
				xp -= levelThreshold * (1.0f + ratio);
				ratio = 0.0f;
				++level;
				if (EnableXPPerRank) {
					IncreasePlayerCharacterXP(level);
				}
			}
			else {
				ratio += xp / levelThreshold;
				xp = 0.0f;
			}
		}

		const bool levelIncreased = level > static_cast<std::int32_t>(Level->value);

		Level->value = static_cast<float>(level);
		Ratio->value = ratio;

		if (levelIncreased) {
			SkillIncreaseEvent ev{ .skillID = ID.data() };
			SkillIncreaseEventSource::Get()->SendEvent(&ev);
			CustomSkillIncreaseRegSet::Get()->SendEvent(ID);
			if (!a_hideNotification) {
				Game::ShowSkillIncreasedMessage(GetName(), level);
			}
		}
	}

	void Skill::Advance(float a_magnitude)
	{
		Advance(a_magnitude, AdvanceObject, true, false);
	}

	void Skill::Increment(std::uint32_t a_count)
	{
		if (a_count == 0 || !Level || !Info || !Info->skill)
			return;

		const float improveMult = Info->skill->improveMult;
		const float improveOffset = Info->skill->improveOffset;

		std::int32_t level = static_cast<std::int32_t>(Level->value);
		if (level >= MaxLevel()) {
			return;
		}

		const std::uint32_t count = (std::min)(a_count, std::uint32_t(MaxLevel()) - level);
		if (Ratio) {
			for (std::uint32_t i = 0; i < count; ++i) {
				const float ratio = (std::min)(Ratio->value, 1.0f);
				const float xp = CalcLevelThreshold(level, improveMult, improveOffset) *
					(1.0f - ratio);
				Advance(xp, AdvanceObject, false, i < count - 1);
				level = static_cast<std::int32_t>(Level->value);
			}
		}
		else {
			for (std::uint32_t i = 0; i < count; ++i) {
				++level;
				if (EnableXPPerRank) {
					IncreasePlayerCharacterXP(level);
				}
			}
			Level->value = static_cast<float>(level);

			SkillIncreaseEvent ev{ .skillID = ID.data() };
			SkillIncreaseEventSource::Get()->SendEvent(&ev);
			CustomSkillIncreaseRegSet::Get()->SendEvent(ID);
			Game::ShowSkillIncreasedMessage(GetName(), level);
		}
	}

	bool Skill::UpdateColor()
	{
		if (!Color) {
			return false;
		}

		auto cv = static_cast<std::int32_t>(Color->value) & 0xFFFFFF;
		if (cv == ColorLast) {
			return true;
		}

		ColorLast = cv;
		ColorStr = fmt::format("#{:06X}"sv, cv);
		return true;
	}
}
