#include "Skill.h"

namespace CustomSkills
{
	static float CalcLevelThreshold(
		std::int32_t a_currentRank,
		float a_improveMult,
		float a_improveOffset)
	{
		static const RE::Setting* const
			fSkillUseCurve = RE::GameSettingCollection::GetSingleton()->GetSetting(
				"fSkillUseCurve");

		const float useCurve = fSkillUseCurve ? fSkillUseCurve->GetFloat() : 1.95f;

		return std::fma(
			std::powf(static_cast<float>(a_currentRank), useCurve),
			a_improveMult,
			a_improveOffset);
	}

	static void IncreasePlayerCharacterXP(std::int32_t a_rankGained)
	{
		static const RE::Setting* const
			fXPPerSkillRank = RE::GameSettingCollection::GetSingleton()->GetSetting(
				"fXPPerSkillRank");

		const float xpPerSkillRank = fXPPerSkillRank ? fXPPerSkillRank->GetFloat() : 1.0f;

		const auto player = RE::PlayerCharacter::GetSingleton();
		player->skills->data->xp += a_rankGained * xpPerSkillRank;
	}

	void Skill::Advance(float a_magnitude, bool a_isSkillUse, bool a_hideNotification)
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
		player->advanceObject = AdvanceObject;
		player->advanceAction = 0;
		RE::BGSEntryPoint::HandleEntryPoint(
			RE::BGSEntryPoint::ENTRY_POINT::kModSkillUse,
			player,
			&xp);
		player->advanceObject = nullptr;

		std::int32_t level = static_cast<std::int32_t>(Level->value);
		float ratio = (std::min)((std::max)(Ratio->value, 0.0f), 1.0f);

		while (level < 100) {
			const float levelThreshold = CalcLevelThreshold(level, improveMult, improveOffset);

			if (ratio + xp / levelThreshold >= 1.0f) {
				xp -= levelThreshold * (1.0f + ratio);
				ratio = 0.0f;
				++level;
				if (EnableXPPerRank) {
					IncreasePlayerCharacterXP(level);
				}
			}
			else {
				ratio += xp / levelThreshold;
				break;
			}
		}

		const bool levelIncreased = level > static_cast<std::int32_t>(Level->value);

		Level->value = static_cast<float>(level);
		Ratio->value = ratio;

		if (!a_hideNotification && levelIncreased) {
			Game::ShowSkillIncreasedMessage(GetName(), level);
		}
	}

	void Skill::Increment(std::uint32_t a_count)
	{
		if (a_count == 0 || !Info || !Info->skill)
			return;

		const float improveMult = Info->skill->improveMult;
		const float improveOffset = Info->skill->improveOffset;

		if (Level) {
			std::int32_t level = static_cast<std::int32_t>(Level->value);
			if (level >= 100) {
				return;
			}

			const std::uint32_t count = (std::min)(a_count, 100U - level);
			if (Ratio) {
				for (std::uint32_t i = 0; i < count; ++i) {
					const float xp = CalcLevelThreshold(level, improveMult, improveOffset) *
						(1.0f - Ratio->value);
					Advance(xp, false, i < count - 1);
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
				Game::ShowSkillIncreasedMessage(GetName(), level);
			}
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
		ColorStr = fmt::format("#{:06X}", cv);
		return true;
	}
}
