#pragma once

namespace CustomSkills
{
	class Scaleform final
	{
	public:
		static void WriteHooks();
	};

	template <std::size_t N>
	struct StringLiteral
	{
		constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

		constexpr operator const char*() const { return value; }

		char value[N];
	};

	template <StringLiteral Prototype, StringLiteral Fn>
	class ScaleformHook final
	{
	public:
		static bool Install(RE::GFxMovieView* a_view);

	private:
		class Func : public RE::GFxFunctionHandler
		{
		public:
			Func(const RE::GFxValue& a_oldFunc) : _oldFunc{ a_oldFunc } {}

			void Call(Params& a_params) override;

		private:
			RE::GFxValue _oldFunc;
		};
	};

	template <StringLiteral Prototype, StringLiteral Fn>
	inline bool ScaleformHook<Prototype, Fn>::Install(RE::GFxMovieView* a_view)
	{
		assert(a_view);

		RE::GFxValue obj;
		a_view->GetVariable(&obj, Prototype);
		if (!obj.IsObject()) {
			return false;
		}

		RE::GFxValue func_old;
		obj.GetMember(Fn, &func_old);
		if (!func_old.IsObject()) {
			return false;
		}

		RE::GFxValue func;
		auto impl = RE::make_gptr<Func>(func_old);

		a_view->CreateFunction(&func, impl.get());
		obj.SetMember(Fn, func);
		return true;
	}
}
