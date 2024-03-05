#include "Skill.h"

namespace CustomSkills
{
	static void IncreasePlayerCharacterXP(std::int32_t a_rankGained)
	{
		static const RE::Setting* const
			fXPPerSkillRank = RE::GameSettingCollection::GetSingleton()->GetSetting(
				"fXPPerSkillRank");

		const float xpPerSkillRank = fXPPerSkillRank ? fXPPerSkillRank->GetFloat() : 1.0f;

		const auto player = RE::PlayerCharacter::GetSingleton();
		player->skills->data->xp += a_rankGained * xpPerSkillRank;
	}

	void Skill::Advance(float a_magnitude)
	{
		if (!Level || !Ratio || !Info || !Info->skill)
			return;

		const float useMult = Info->skill->useMult;
		if (useMult == 0.0f)
			return;
		const float useOffset = Info->skill->useOffset;
		const float improveMult = Info->skill->improveMult;
		const float improveOffset = Info->skill->improveOffset;

		static const RE::Setting* const
			fSkillUseCurve = RE::GameSettingCollection::GetSingleton()->GetSetting(
				"fSkillUseCurve");

		const float useCurve = fSkillUseCurve ? fSkillUseCurve->GetFloat() : 1.95f;

		float xp = std::fma(a_magnitude, useMult, useOffset);
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
			float levelThreshold = std::fma(
				std::powf(static_cast<float>(level), useCurve),
				improveMult,
				improveOffset);

			if (ratio + xp / levelThreshold >= 1.0f) {
				xp -= levelThreshold * (1.0f + ratio);
				++level;
				IncreasePlayerCharacterXP(level);
				ratio = 0.0f;
			}
			else {
				ratio += xp / levelThreshold;
				break;
			}
		}

		const bool levelIncreased = level > static_cast<std::int32_t>(Level->value);

		Level->value = static_cast<float>(level);
		Ratio->value = ratio;

		if (levelIncreased) {
			Game::ShowSkillIncreasedMessage(GetName(), level);
		}
	}

	void Skill::Increment(std::uint32_t a_count)
	{
		if (a_count == 0)
			return;

		if (Level) {
			if (Level->value >= 100.0f) {
				return;
			}

			for (; a_count && Level->value < 100.0f; --a_count) {
				const std::int32_t rankGained = static_cast<std::int32_t>(Level->value) + 1;
				Level->value = static_cast<float>(rankGained);
				IncreasePlayerCharacterXP(rankGained);
			}

			Game::ShowSkillIncreasedMessage(GetName(), static_cast<std::int32_t>(Level->value));
		}

		if (Ratio) {
			Ratio->value = 0.0f;
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
