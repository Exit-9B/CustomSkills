#pragma once

namespace CustomSkills
{
	struct CImageController
	{
		enum class State
		{
			Resting = 0,
			Entering = 1,
			Exiting = 2,
		};

		static std::uint32_t CurrentTime()
		{
			static REL::Relocation<std::uint32_t*> currentTime{ REL::ID(410201) };
			return *currentTime;
		}

		bool IsActive()
		{
			return state != State::Resting || shader && shader->GetAlpha() > 0.0f;
		}

		void SetShader(RE::BSShaderProperty* a_shader)
		{
			shader = a_shader;
			shader->SetMaterial(a_shader->material, true);
			shader->SetAlpha(0.0f);
		}

		void Enter()
		{
			stateChangeTime = CurrentTime();
			state = State::Entering;
		}

		void Exit()
		{
			if (IsActive()) {
				stateChangeTime = CurrentTime();
				state = State::Exiting;
			}
		}

		void Update()
		{
			if (!shader || state == State::Resting)
				return;

			if (CurrentTime() - stateChangeTime <= 500) {
				float alpha = (CurrentTime() - stateChangeTime) / 500.0f;
				if (state != State::Entering) {
					alpha = 1.0f - alpha;
				}

				shader->SetAlpha(alpha);
			}
			else {
				float alpha = state == State::Entering ? 1.0f : 0.0f;
				shader->SetAlpha(alpha);

				state = State::Resting;
			}
		}

		RE::BSShaderProperty* shader = nullptr;
		SKSE::stl::enumeration<State, std::uint32_t> state = State::Resting;
		std::uint32_t stateChangeTime;
	};
	static_assert(sizeof(CImageController) == 0x10);
}
