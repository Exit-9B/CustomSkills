#include "CustomSkills/CustomSkillsInterface.h"
#include "CustomSkills/CustomSkillsManager.h"
#include "CustomSkills/Hooks/ActorValue.h"
#include "CustomSkills/Hooks/BeastSkillInfo.h"
#include "CustomSkills/Hooks/Constellation.h"
#include "CustomSkills/Hooks/Legendary.h"
#include "CustomSkills/Hooks/MenuSetup.h"
#include "CustomSkills/Hooks/Navigation.h"
#include "CustomSkills/Hooks/Scaleform.h"
#include "CustomSkills/Hooks/SkillBooks.h"
#include "CustomSkills/Hooks/SkillInfo.h"
#include "CustomSkills/Hooks/SkillProgress.h"
#include "CustomSkills/Hooks/SkillUse.h"
#include "CustomSkills/Hooks/Training.h"
#include "CustomSkills/Hooks/Update.h"
#include "CustomSkills/Serialization.h"
#include "Papyrus/CustomSkills.h"
#include "Papyrus/Events.h"

namespace
{
	void InitializeLog()
	{
#ifndef NDEBUG
		auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
		auto path = logger::log_directory();
		if (!path) {
			util::report_and_fail("Failed to find standard logging directory"sv);
		}

		*path /= fmt::format("{}.log"sv, Plugin::NAME);
		auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

#ifndef NDEBUG
		const auto level = spdlog::level::trace;
#else
		const auto level = spdlog::level::info;
#endif

		auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));
		log->set_level(level);
		log->flush_on(level);

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("%s(%#): [%^%l%$] %v"s);
	}
}

extern "C" DLLEXPORT constinit auto SKSEPlugin_Version = []()
{
	SKSE::PluginVersionData v{};

	v.PluginVersion(Plugin::VERSION);
	v.PluginName(Plugin::NAME);
	v.AuthorName("meh321 and Parapets"sv);

	v.UsesAddressLibrary(true);

	return v;
}();

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	logger::info("{} v{}"sv, Plugin::NAME, Plugin::VERSION.string());

	SKSE::Init(a_skse);
	SKSE::AllocTrampoline(242);

	using namespace CustomSkills;

	CustomSkillsManager::Initialize();

	Update::WriteHooks();
	ActorValue::WriteHooks();
	MenuSetup::WriteHooks();
	Navigation::WriteHooks();
	Constellation::WriteHooks();
	SkillInfo::WriteHooks();
	SkillProgress::WriteHooks();
	Legendary::WriteHooks();
	BeastSkillInfo::WriteHooks();

	Training::WriteHooks();
	SkillBooks::WriteHooks();
	SkillUse::WriteHooks();

	auto* const papyrus = SKSE::GetPapyrusInterface();
	papyrus->Register(
		[](RE::BSScript::IVirtualMachine* a_vm)
		{
			Papyrus::CustomSkills::RegisterFuncs(a_vm);
			Papyrus::Events<RE::TESForm>::RegisterFuncs(a_vm);
			Papyrus::Events<RE::BGSBaseAlias>::RegisterFuncs(a_vm);
			Papyrus::Events<RE::ActiveEffect>::RegisterFuncs(a_vm);

			return true;
		});

	auto* const serialization = SKSE::GetSerializationInterface();
	serialization->SetUniqueID(Serialization::ID);
	serialization->SetLoadCallback(&Serialization::LoadCallback);
	serialization->SetRevertCallback(&Serialization::RevertCallback);
	serialization->SetSaveCallback(&Serialization::SaveCallback);

	auto* const messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(
		[](auto a_msg)
		{
			switch (a_msg->type) {
			case SKSE::MessagingInterface::kInputLoaded:
				Scaleform::WriteHooks();
				break;
			case SKSE::MessagingInterface::kDataLoaded:
				CustomSkillsManager::LoadSkills();
				CustomSkills::Impl::CustomSkillsInterface::Dispatch();
				break;
			}
		});

	return true;
}
