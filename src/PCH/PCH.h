#pragma once

#include <RE/Skyrim.h>
#include <SKSE/SKSE.h>

#ifdef NDEBUG
#include <spdlog/sinks/basic_file_sink.h>
#else
#include <spdlog/sinks/msvc_sink.h>
#endif

using namespace std::literals;

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

	inline bool starts_with(const std::string& a_str, const std::string& a_subStr)
	{
		return a_str.rfind(a_subStr, 0) == 0;
	}

	inline bool ends_with(const std::string& a_str, const std::string& a_subStr)
	{
		if (a_str.length() < a_subStr.length()) {
			return false;
		}

		return a_str.compare(a_str.length() - a_subStr.length(), a_subStr.length(), a_subStr) == 0;
	}

	inline std::string toupper(std::string s)
	{
		std::ranges::transform(
			s,
			s.begin(),
			[](unsigned char c)
			{
				return static_cast<char>(std::toupper(c));
			});
		return s;
	}
}

#define DLLEXPORT __declspec(dllexport)

#include "Plugin.h"
