#include "Scaleform.h"

#include "CustomSkills/CustomSkillsManager.h"

namespace CustomSkills
{
	constexpr StringLiteral AnimatedSkillText = "_global.AnimatedSkillText.prototype";

	void Scaleform::WriteHooks()
	{
		auto hook = REL::Relocation<std::uintptr_t>(
			RE::Offset::BSScaleformManager::LoadMovie,
			0x1DD);

		if (!REL::make_pattern<"FF 15">().match(hook.address())) {
			util::report_and_fail("Failed to install Scaleform hook");
		}

		using SetViewScaleMode_t = void(RE::GFxMovieView*, RE::GFxMovieView::ScaleModeType);
		static REL::Relocation<SetViewScaleMode_t> _SetViewScaleMode;

		auto AddScaleformHooks =
			+[](RE::GFxMovieView* a_view, RE::GFxMovieView::ScaleModeType a_scaleMode)
		{
			_SetViewScaleMode(a_view, a_scaleMode);

			if (ScaleformHook<AnimatedSkillText, "SetAngle">::Install(a_view)) {
				a_view->SetVariable(
					"_global.AnimatedSkillText.prototype.SKILLS",
					CustomSkillsManager::GetCurrentSkillCount());
			}
		};

		// TRAMPOLINE: 8
		auto& trampoline = SKSE::GetTrampoline();
		const auto ptr = trampoline.write_call<6>(hook.address(), AddScaleformHooks);
		_SetViewScaleMode = *reinterpret_cast<std::uintptr_t*>(ptr);
	}

	template <>
	void ScaleformHook<AnimatedSkillText, "SetAngle">::Func::Call(Params& a_params)
	{
		if (a_params.argCount < 1)
			return;

		[[maybe_unused]] const RE::GFxValue& aAngle = a_params.args[0];

		const std::int32_t numSkills = CustomSkillsManager::GetCurrentSkillCount();
		const double skillAngle = 360.0 / numSkills;

		const RE::GFxMovieView* const view = static_cast<RE::GFxMovieView*>(a_params.movie);
		const RE::GRectF visibleRect = view->GetVisibleFrameRect();
		const float center = 640.0f;
		const float halfWidth = (visibleRect.right - visibleRect.left) * 0.5f;

		double spacing = halfWidth / (numSkills / 2.0 + 0.75);
		double midSpacing = spacing * 0.75;

		spacing = (std::max)(140.0, spacing);
		midSpacing = (std::max)(90.0, midSpacing);

		const double lookingAtPos = aAngle.GetNumber() / skillAngle;
		double lookingAtPosI;
		const double fracOffset = std::modf(lookingAtPos, &lookingAtPosI);
		const int lookingAtIndex = static_cast<int>(lookingAtPosI);
		static constexpr double bias = 0.1;

		for (std::int32_t i = 0; i < numSkills; ++i) {
			RE::GFxValue SkillText;
			a_params.thisPtr->GetMember(fmt::format("SkillText{}", i).data(), &SkillText);
			if (!SkillText.IsDisplayObject())
				continue;

			int offset = i - lookingAtIndex;
			if (offset - fracOffset > numSkills / 2.0 + bias) {
				offset -= numSkills;
			}
			else if (offset - fracOffset <= -numSkills / 2.0 + bias) {
				offset += numSkills;
			}

			int offset0 = offset - 1;
			double xMin = center + offset0 * spacing;
			double xMax = center + offset * spacing;
			xMin += offset0 > 0 ? midSpacing : offset0 < 0 ? -midSpacing : 0.0;
			xMax += offset > 0 ? midSpacing : offset < 0 ? -midSpacing : 0.0;

			SkillText.SetMember("_x", std::lerp(xMin, xMax, 1.0 - fracOffset));

			double scale = 100.0;
			double barScale = 100.0;
			if (offset == 0) {
				scale += (1.0 - fracOffset) * 75.0;
				barScale -= (1.0 - fracOffset) * 30.0;
			}
			else if (offset == 1) {
				scale += fracOffset * 75.0;
				barScale -= fracOffset * 30.0;
			}

			SkillText.SetMember("_xscale", scale);
			SkillText.SetMember("_yscale", scale);

			RE::GFxValue ShortBar;
			SkillText.GetMember("ShortBar", &ShortBar);
			if (ShortBar.IsDisplayObject()) {
				ShortBar.SetMember("_yscale", barScale);
			}
		}
	}
}
