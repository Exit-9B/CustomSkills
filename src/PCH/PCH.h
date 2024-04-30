#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;
using namespace RE::literals;

namespace logger = SKSE::log;

namespace util
{
	using SKSE::stl::report_and_fail;
	using SKSE::stl::to_underlying;

	template <typename T>
	inline void write_disp(std::uintptr_t a_dst, std::uintptr_t a_rip, REL::Relocation<T*>& a_var)
	{
		const auto disp = reinterpret_cast<const std::byte*>(a_var.get()) -
			reinterpret_cast<const std::byte*>(a_rip);

		REL::safe_write(a_dst, static_cast<std::int32_t>(disp));
	}

	inline void write_14branch(std::uintptr_t a_src, const void* a_code)
	{
#pragma pack(push, 1)
		struct Assembly
		{
			// jmp [rip]
			std::uint8_t jmp;    // 0 - 0xFF
			std::uint8_t modrm;  // 1 - 0x25
			std::int32_t disp;   // 2 - 0x00000000
			std::uint64_t addr;  // 6 - [rip]
		};
		static_assert(offsetof(Assembly, jmp) == 0x0);
		static_assert(offsetof(Assembly, modrm) == 0x1);
		static_assert(offsetof(Assembly, disp) == 0x2);
		static_assert(offsetof(Assembly, addr) == 0x6);
		static_assert(sizeof(Assembly) == 0xE);
#pragma pack(pop)

		Assembly mem{
			.jmp = 0xFF,
			.modrm = 0x25,
			.disp = 0x00000000,
			.addr = std::bit_cast<std::uint64_t>(a_code),
		};

		REL::safe_write(a_src, std::addressof(mem), sizeof(mem));
	}

	[[nodiscard]] constexpr int ascii_tolower(int ch) noexcept
	{
		if (ch >= 'A' && ch <= 'Z')
			ch += 'a' - 'A';
		return ch;
	}

	struct iless
	{
		using is_transparent = int;

		template <std::ranges::contiguous_range S1, std::ranges::contiguous_range S2>
			requires(
				std::is_same_v<std::ranges::range_value_t<S1>, char> &&
				std::is_same_v<std::ranges::range_value_t<S2>, char>)
		constexpr bool operator()(S1&& a_str1, S2&& a_str2) const
		{
			std::size_t count = std::ranges::size(a_str2);
			const std::size_t len1 = std::ranges::size(a_str1);
			const bool shorter = len1 < count;
			if (shorter)
				count = len1;

			if (count) {
				const char* p1 = std::ranges::data(a_str1);
				const char* p2 = std::ranges::data(a_str2);

				do {
					const int ch1 = ascii_tolower(*p1++);
					const int ch2 = ascii_tolower(*p2++);
					if (ch1 != ch2)
						return ch1 < ch2;
				} while (--count);
			}

			return shorter;
		}

	};

	template <typename T, typename Allocator = std::allocator<std::pair<const std::string, T>>>
	using istring_map = std::map<std::string, T, iless, Allocator>;

	inline std::optional<RE::ActorValue> ParseSkill(std::string_view a_name)
	{
		static constexpr auto SKILLS = std::to_array<std::pair<std::string_view, RE::ActorValue>>({
			{ "Alchemy"sv, RE::ActorValue::kAlchemy },
			{ "Alteration"sv, RE::ActorValue::kAlteration },
			{ "Block"sv, RE::ActorValue::kBlock },
			{ "Conjuration"sv, RE::ActorValue::kConjuration },
			{ "Destruction"sv, RE::ActorValue::kDestruction },
			{ "Enchanting"sv, RE::ActorValue::kEnchanting },
			{ "HeavyArmor"sv, RE::ActorValue::kHeavyArmor },
			{ "Illusion"sv, RE::ActorValue::kIllusion },
			{ "LightArmor"sv, RE::ActorValue::kLightArmor },
			{ "Lockpicking"sv, RE::ActorValue::kLockpicking },
			{ "Marksman"sv, RE::ActorValue::kArchery },
			{ "OneHanded"sv, RE::ActorValue::kOneHanded },
			{ "Pickpocket"sv, RE::ActorValue::kPickpocket },
			{ "Restoration"sv, RE::ActorValue::kRestoration },
			{ "Smithing"sv, RE::ActorValue::kSmithing },
			{ "Sneak"sv, RE::ActorValue::kSneak },
			{ "Speechcraft"sv, RE::ActorValue::kSpeech },
			{ "TwoHanded"sv, RE::ActorValue::kTwoHanded },
			{ "VampirePerks"sv, RE::ActorValue::kVampirePerks },
			{ "WerewolfPerks"sv, RE::ActorValue::kWerewolfPerks },
		});
		static_assert(std::ranges::is_sorted(SKILLS));

		const auto it = std::ranges::lower_bound(
			SKILLS,
			a_name,
			iless{},
			[](auto&& kv)
			{
				return kv.first;
			});

		if (it != std::end(SKILLS) && !iless{}(a_name, it->first)) {
			return it->second;
		}

		return std::nullopt;
	}

}

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
