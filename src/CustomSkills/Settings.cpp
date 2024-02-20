#include "Settings.h"

#include "TreeNode.h"

#include <SimpleIni.h>

namespace CustomSkills
{
	auto Settings::ReadSkills() -> std::map<std::string, std::shared_ptr<Skill>, util::iless>
	{
		std::map<std::string, std::shared_ptr<Skill>, util::iless> skills;

		auto dir = std::filesystem::path("Data/NetScriptFramework/Plugins");
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			auto filename = entry.path().filename().string();

			if (!util::starts_with(util::toupper(filename), "CUSTOMSKILL."s)) {
				continue;
			}

			if (!util::ends_with(util::toupper(filename), ".CONFIG.TXT"s)) {
				continue;
			}

			std::string key = filename.substr(12);
			key = key.substr(0, key.length() - 11);

			if (key.empty())
				continue;

			if (auto sk = ReadSkill(entry.path())) {
				skills.insert({ std::move(key), sk });
			}
		}

		if (ec) {
			logger::error("Error reading skill configs: {}", ec.message());
		}

		return skills;
	}

	auto Settings::ReadSkill(const std::filesystem::path& a_file) -> std::shared_ptr<Skill>
	{
		auto sk = std::make_unique<Skill>();

		::CSimpleIniA cv;
		cv.SetUnicode();
		cv.LoadFile(a_file.string().data());

		auto GetStringValue =
			[&cv](const char* a_section, const char* a_key, const char* a_default) -> std::string
		{
			std::string s = cv.GetValue(a_section, a_key, a_default);
			if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
				s.erase(0, 1);
				s.erase(s.size() - 1);
			}
			return s;
		};

		const auto dataHandler = RE::TESDataHandler::GetSingleton();

		RE::BSString defaultName;
		RE::BSString defaultDescription;
		auto vampireTree = dataHandler->LookupForm<RE::ActorValueInfo>(0x646, "Skyrim.esm");
		if (vampireTree) {
			defaultName = vampireTree->GetFullName();
			vampireTree->GetDescription(defaultDescription, nullptr);
		}

		std::string name = GetStringValue("", "Name", defaultName.c_str());
		std::string description = GetStringValue("", "Description", defaultDescription.c_str());
		SKSE::Translation::Translate(name, name);
		SKSE::Translation::Translate(description, description);

		sk->Name = std::move(name);
		sk->Description = std::move(description);

		sk->Skydome = GetStringValue("", "Skydome", "DLC01/Interface/INTVampirePerkSkydome.nif");
		sk->NormalNif = cv.GetBoolValue("", "SkydomeNormalNif", false);

		auto levelFile = GetStringValue("", "LevelFile", "");
		auto levelId = cv.GetLongValue("", "LevelId", 0x0);
		sk->Level = dataHandler->LookupForm<RE::TESGlobal>(levelId, levelFile);

		auto ratioFile = GetStringValue("", "RatioFile", "");
		auto ratioId = cv.GetLongValue("", "RatioId", 0x0);
		sk->Ratio = dataHandler->LookupForm<RE::TESGlobal>(ratioId, ratioFile);

		auto showLevelupFile = GetStringValue("", "ShowLevelupFile", "");
		auto showLevelupId = cv.GetLongValue("", "ShowLevelupId", 0x0);
		sk->ShowLevelup = dataHandler->LookupForm<RE::TESGlobal>(showLevelupId, showLevelupFile);

		auto showMenuFile = GetStringValue("", "ShowMenuFile", "");
		auto showMenuId = cv.GetLongValue("", "ShowMenuId", 0x0);
		sk->OpenMenu = dataHandler->LookupForm<RE::TESGlobal>(showMenuId, showMenuFile);

		auto perkPointsFile = GetStringValue("", "PerkPointsFile", "");
		auto perkPointsId = cv.GetLongValue("", "PerkPointsId", 0x0);
		sk->PerkPoints = dataHandler->LookupForm<RE::TESGlobal>(perkPointsId, perkPointsFile);

		auto legendaryFile = GetStringValue("", "LegendaryFile", "");
		auto legendaryId = cv.GetLongValue("", "LegendaryId", 0x0);
		sk->Legendary = dataHandler->LookupForm<RE::TESGlobal>(legendaryId, legendaryFile);

		auto colorFile = GetStringValue("", "ColorFile", "");
		auto colorId = cv.GetLongValue("", "ColorId", 0x0);
		sk->Color = dataHandler->LookupForm<RE::TESGlobal>(colorId, colorFile);

		auto debugReloadFile = GetStringValue("", "DebugReloadFile", "");
		auto debugReloadId = cv.GetLongValue("", "DebugReloadId", 0x0);
		sk->DebugReload = dataHandler->LookupForm<RE::TESGlobal>(debugReloadId, debugReloadFile);

		std::vector<std::shared_ptr<TreeNode>> nodes;
		for (std::int32_t i = 0; i < MaxNodes; i++) {
			auto enable = cv.GetBoolValue("", fmt::format("Node{}.Enable", i).c_str());
			if (!enable) {
				continue;
			}

			auto tn = std::make_shared<TreeNode>();
			tn->Index = i;
			tn->PerkFile = GetStringValue("", fmt::format("Node{}.PerkFile", i).c_str(), "");
			tn->PerkId = cv.GetLongValue("", fmt::format("Node{}.PerkId", i).c_str(), 0x0);
			tn->X = (float)cv.GetDoubleValue("", fmt::format("Node{}.X", i).c_str());
			tn->Y = (float)cv.GetDoubleValue("", fmt::format("Node{}.Y", i).c_str());
			tn->GridX = cv.GetLongValue("", fmt::format("Node{}.GridX", i).c_str());
			tn->GridY = cv.GetLongValue("", fmt::format("Node{}.GridY", i).c_str());

			std::string links = GetStringValue("", fmt::format("Node{}.Links", i).c_str(), "");
			if (!links.empty()) {
				const std::string delims = " \t,";
				std::size_t beg, pos = 0;
				while ((beg = links.find_first_not_of(delims, pos)) != std::string::npos) {
					pos = links.find_first_of(delims, beg + 1);
					std::string token = links.substr(beg, pos - beg);

					try {
						tn->Links.push_back(std::stoi(token));
					}
					catch (std::invalid_argument&) {
						Error(
							a_file,
							fmt::format(
								"Format error in Node{}.Links! Unable to parse integer from {}!",
								i,
								token));
						return nullptr;
					}
				}
			}

			nodes.push_back(tn);
		}

		try {
			sk->SkillTree = TreeNode::Create(nodes);
		}
		catch (...) {
			Error(
				a_file,
				"Something went wrong when creating skill perk tree! Make sure node 0 exists and "
				"no missing nodes are referenced in links.");
		}

		return sk;
	}

	void Settings::Error(const std::filesystem::path& a_file, std::string_view a_message)
	{
		logger::error(
			"CustomSkills plugin: Failed to read skill from file `{}`: {}",
			a_file.string(),
			a_message);
	}
}
