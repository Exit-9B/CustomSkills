#include "Settings.h"

#include "RE/Offset.h"
#include "TreeNode.h"

#include <SimpleIni.h>
#include <json/json.h>

namespace CustomSkills
{
	auto Settings::ReadSkills() -> util::istring_map<std::shared_ptr<SkillGroup>>
	{
		util::istring_map<std::shared_ptr<SkillGroup>> skills;

		auto dir = std::filesystem::path("Data/SKSE/Plugins/CustomSkills"sv);
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename();

			if (::_wcsicmp(filename.extension().c_str(), L".json") != 0) {
				continue;
			}

			std::string key = filename.stem().string();

			if (const auto group = ReadSkills(entry.path())) {
				if (::_stricmp(key.data(), "SKILLS") == 0) {
					static constinit bool firstInit = true;
					const auto count = static_cast<std::uint32_t>(group->Skills.size());
					if (firstInit) {
						// starting value is 15, centered on mage?
						const auto tree = static_cast<std::uint32_t>(0.5 + count * (2.5 / 3.0));
						if (tree < count)
							group->LastSelectedTree = tree;
						firstInit = false;
					}
					else {
						REL::Relocation<std::uint32_t*> lastSelectedTree{
							RE::Offset::StatsMenu::LastSelectedTree
						};

						if (*lastSelectedTree < count) {
							group->LastSelectedTree = *lastSelectedTree;
						}
					}
				}

				skills.emplace(std::move(key), std::move(group));
			}
		}

		if (ec) {
			logger::error("Error reading skill configs: {}"sv, ec.message());
		}

		dir = std::filesystem::path("Data/NetScriptFramework/Plugins"sv);
		ec = {};
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			const auto filename = entry.path().filename();

			constexpr auto prefix = L"CustomSkill."sv;
			if (::_wcsnicmp(filename.c_str(), prefix.data(), prefix.size()) != 0) {
				continue;
			}

			if (::_wcsicmp(filename.extension().c_str(), L".txt") != 0 ||
				::_wcsicmp(filename.stem().extension().c_str(), L".config") != 0) {
				continue;
			}

			std::string key = filename.stem().stem().string().substr(prefix.size());

			if (key.empty())
				continue;

			if (const auto sk = ReadLegacySkill(entry.path())) {
				skills.emplace(std::move(key), std::move(sk));
			}
		}

		if (ec) {
			logger::error("Error reading legacy skill configs: {}"sv, ec.message());
		}

		return skills;
	}

	template <typename T>
	static T* ParseForm(RE::TESDataHandler* dataHandler, const Json::Value& a_val)
	{
		std::istringstream ss{ a_val.asString() };
		std::string file;
		std::string id;

		std::getline(ss, file, '|');
		std::getline(ss, id);
		RE::FormID rawFormID;
		std::istringstream(id) >> std::hex >> rawFormID;

		return dataHandler->LookupForm<T>(rawFormID, file);
	}

	static RE::TESForm* CreateAdvanceObject(RE::BGSKeyword* a_keyword)
	{
		const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::TESObjectACTI>();
		const auto obj = factory ? factory->Create() : nullptr;
		if (obj && a_keyword) {
			obj->keywords = RE::calloc<RE::BGSKeyword*>(1);
			obj->keywords[0] = a_keyword;
			obj->numKeywords = 1;
		}
		return obj;
	}

	static auto ReadSkillDef(const Json::Value& skill) -> std::shared_ptr<Skill>
	{
		if (const auto uri = skill["$ref"s].asString(); !uri.empty()) {
			Json::Value external;
			const auto path = uri.front() == '/'
				? std::filesystem::path("Data"sv) / uri
				: std::filesystem::path("Data/SKSE/Plugins/CustomSkills/"sv) / uri;

			if (auto stream = std::ifstream(path.string()); stream.good()) {
				try {
					stream >> external;
				}
				catch (...) {
					logger::error("Parse errors in file: {}"sv, path.string());
				}
			}

			if (external.isObject()) {
				return ReadSkillDef(external);
			}
		}

		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler)
			return nullptr;

		const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::ActorValueInfo>();
		if (!factory)
			return nullptr;

		const auto sk = std::make_shared<Skill>();
		sk->Info = factory->Create();

		if (const auto& id = skill["id"s]; id.isString()) {
			sk->ID = id.asString();
			if (const auto advanceKeyword = RE::TESForm::LookupByEditorID<RE::BGSKeyword>(
					fmt::format("CustomSkillAdvance_{}"sv, sk->ID))) {
				sk->AdvanceObject = CreateAdvanceObject(advanceKeyword);
			}
		}

		if (const auto& name = skill["name"s]; name.isString()) {
			std::string tr = name.asString();
			SKSE::Translation::Translate(tr, tr);
			sk->Info->fullName = tr;
		}

		if (const auto& description = skill["description"s]; description.isString()) {
			std::string tr = description.asString();
			SKSE::Translation::Translate(tr, tr);
			sk->Description = std::move(tr);
		}

		if (const auto& level = skill["level"s]; level.isString()) {
			sk->Level = ParseForm<RE::TESGlobal>(dataHandler, level);
		}

		if (const auto& ratio = skill["ratio"s]; ratio.isString()) {
			sk->Ratio = ParseForm<RE::TESGlobal>(dataHandler, ratio);
		}

		if (const auto& legendary = skill["legendary"s]; legendary.isString()) {
			sk->Legendary = ParseForm<RE::TESGlobal>(dataHandler, legendary);
		}

		if (const auto& color = skill["color"s]; color.isString()) {
			sk->Color = ParseForm<RE::TESGlobal>(dataHandler, color);
		}

		if (const auto& showLevelup = skill["showLevelup"s]; showLevelup.isString()) {
			sk->ShowLevelup = ParseForm<RE::TESGlobal>(dataHandler, showLevelup);
		}

		sk->Info->skill = new RE::ActorValueInfo::Skill{
			.useMult = 1.0f,
			.useOffset = 0.0f,
			.improveMult = 1.0f,
			.improveOffset = 0.0f
		};

		if (const auto& experienceFormula = skill["experienceFormula"s];
			experienceFormula.isObject()) {
			if (const auto& useMult = experienceFormula["useMult"s]; useMult.isNumeric()) {
				sk->Info->skill->useMult = useMult.asFloat();
			}
			if (const auto& useOffset = experienceFormula["useOffset"s]; useOffset.isNumeric()) {
				sk->Info->skill->useOffset = useOffset.asFloat();
			}
			if (const auto& improveMult = experienceFormula["improveMult"s];
				improveMult.isNumeric()) {
				sk->Info->skill->improveMult = improveMult.asFloat();
			}
			if (const auto& improveOffset = experienceFormula["improveOffset"s];
				improveOffset.isNumeric()) {
				sk->Info->skill->improveOffset = improveOffset.asFloat();
			}
			if (const auto& enableXPPerRank = experienceFormula["enableXPPerRank"s];
				enableXPPerRank.isBool()) {
				sk->EnableXPPerRank = enableXPPerRank.asBool();
			}
		}

		std::vector<std::shared_ptr<TreeNode>> tns;
		tns.emplace_back(std::make_shared<TreeNode>())->Index = 0;
		if (const auto& nodes = skill["nodes"s]; nodes.isArray()) {
			std::map<std::string, std::int32_t> ids;

			const std::size_t size = (std::min)(127U, nodes.size());
			tns.reserve(size + 1);
			for (std::int32_t i = 0; i < size; ++i) {
				tns[0]->Links.push_back(i + 1);

				const auto& node = nodes[i];
				if (const auto& id = node["id"s]; id.isString()) {
					ids.emplace(id.asString(), i + 1);
				}
			}

			std::int32_t gridWidth = 1;
			for (const auto& node : nodes) {
				const auto x = node["x"s].asDouble();
				gridWidth = (std::max)(gridWidth, static_cast<std::int32_t>(std::ceil(x * 2)));
			}

			sk->Info->perkTreeWidth = gridWidth;
			const std::uint32_t xMax = (gridWidth + 1) / 2;

			for (std::int32_t i = 0; i < size; ++i) {
				const auto& node = nodes[i];
				const auto& tn = tns.emplace_back(std::make_shared<TreeNode>());
				tn->Index = i + 1;
				if (const auto& perk = node["perk"s]; perk.isString()) {
					tn->Perk = ParseForm<RE::BGSPerk>(dataHandler, perk);
				}

				const auto x = node["x"s].asDouble() + gridWidth * 0.5;
				const auto y = node["y"s].asDouble();
				tn->GridX = (std::max)(static_cast<std::int32_t>(x + 0.58), 0);
				tn->X = static_cast<float>(x - tn->GridX);
				tn->GridY = (std::min)(
					(std::max)(static_cast<std::int32_t>(y - 0.12 * (xMax + 1)), 0),
					4);
				tn->Y = static_cast<float>(y - tn->GridY);

				if (const auto& links = node["links"s]; links.isArray()) {
					for (const auto& link : links) {
						if (link.isInt()) {
							// index is 1-based, do not modify
							const int id = link.asInt();
							tn->Links.push_back(id);
							std::erase(tns[0]->Links, id);
						}
						else if (link.isString()) {
							const int id = ids[link.asString()];
							tn->Links.push_back(id);
							std::erase(tns[0]->Links, id);
						}
					}
				}
			}
		}

		sk->Info->perkTree = TreeNode::Create(tns, sk->Info);
		return sk;
	}

	auto Settings::ReadSkills(const std::filesystem::path& a_file) -> std::shared_ptr<SkillGroup>
	{
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler)
			return nullptr;

		Json::Value root;
		if (auto fileStream = std::ifstream(a_file.string()); fileStream.good()) {
			try {
				fileStream >> root;
			}
			catch (...) {
				logger::error("Parse errors in file: {}"sv, a_file.string());
			}
		}

		if (!root.isObject())
			return nullptr;

		auto&& group = std::make_shared<SkillGroup>();

		if (const auto& skydome = root["skydome"s]; skydome.isObject()) {
			group->Skydome = skydome["model"s].asString();
			if (const auto& cameraRightPoint = skydome["cameraRightPoint"s];
				cameraRightPoint.isUInt()) {
				group->CameraRightPoint = cameraRightPoint.asUInt();
			}
		}

		if (group->Skydome.empty()) {
			group->Skydome = "DLC01/Interface/INTVampirePerkSkydome.nif"s;
		}

		if (const auto& showMenu = root["showMenu"s]; showMenu.isString()) {
			group->OpenMenu = ParseForm<RE::TESGlobal>(dataHandler, showMenu);
		}

		if (const auto& debugReload = root["debugReload"s]; debugReload.isString()) {
			group->DebugReload = ParseForm<RE::TESGlobal>(dataHandler, debugReload);
		}

		if (const auto& perkPoints = root["perkPoints"s]; perkPoints.isString()) {
			group->PerkPoints = ParseForm<RE::TESGlobal>(dataHandler, perkPoints);
		}

		if (const auto& skills = root["skills"s]; skills.isArray()) {
			auto actorValue = CUSTOM_SKILL_BASE_VALUE;
			for (const auto& skill : skills) {
				if (skill.isString()) {
					group->Skills.emplace_back(nullptr);
					group->ActorValues.push_back(
						util::ParseSkill(skill.asString()).value_or(RE::ActorValue::kHealth));
					++actorValue;
					continue;
				}
				else if (!skill.isObject()) {
					continue;
				}

				try {
					if (auto&& sk = ReadSkillDef(skill)) {
						group->Skills.push_back(std::move(sk));
						group->ActorValues.push_back(static_cast<RE::ActorValue>(actorValue++));
					}
				}
				catch (...) {
					Error(
						a_file,
						"Something went wrong when creating skill perk tree! Make sure that no "
						"missing nodes are referenced in links."sv);
					return nullptr;
				}
			}
		}

		return group;
	}

	auto Settings::ReadLegacySkill(const std::filesystem::path& a_file)
		-> std::shared_ptr<SkillGroup>
	{
		auto group = std::make_shared<SkillGroup>();
		const auto& sk = group->Skills.emplace_back(std::make_shared<Skill>());
		group->ActorValues.push_back(static_cast<RE::ActorValue>(CUSTOM_SKILL_BASE_VALUE));

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
		if (!dataHandler)
			return nullptr;

		const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::ActorValueInfo>();
		sk->Info = factory ? factory->Create() : nullptr;
		if (!sk->Info) {
			return nullptr;
		}

		RE::BSString defaultName;
		RE::BSString defaultDescription;
		auto vampireTree = dataHandler->LookupForm<RE::ActorValueInfo>(0x646, "Skyrim.esm"sv);
		if (vampireTree) {
			defaultName = vampireTree->GetFullName();
			vampireTree->GetDescription(defaultDescription, nullptr);
		}

		std::string name = GetStringValue("", "Name", defaultName.c_str());
		std::string description = GetStringValue("", "Description", defaultDescription.c_str());
		SKSE::Translation::Translate(name, name);
		SKSE::Translation::Translate(description, description);

		sk->Info->fullName = name;
		sk->Description = std::move(description);

		group
			->Skydome = GetStringValue("", "Skydome", "DLC01/Interface/INTVampirePerkSkydome.nif");
		group->CameraRightPoint = cv.GetBoolValue("", "SkydomeNormalNif", false) ? 1 : 2;

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
		group->OpenMenu = dataHandler->LookupForm<RE::TESGlobal>(showMenuId, showMenuFile);

		auto perkPointsFile = GetStringValue("", "PerkPointsFile", "");
		auto perkPointsId = cv.GetLongValue("", "PerkPointsId", 0x0);
		group->PerkPoints = dataHandler->LookupForm<RE::TESGlobal>(perkPointsId, perkPointsFile);

		auto legendaryFile = GetStringValue("", "LegendaryFile", "");
		auto legendaryId = cv.GetLongValue("", "LegendaryId", 0x0);
		sk->Legendary = dataHandler->LookupForm<RE::TESGlobal>(legendaryId, legendaryFile);

		auto colorFile = GetStringValue("", "ColorFile", "");
		auto colorId = cv.GetLongValue("", "ColorId", 0x0);
		sk->Color = dataHandler->LookupForm<RE::TESGlobal>(colorId, colorFile);

		auto debugReloadFile = GetStringValue("", "DebugReloadFile", "");
		auto debugReloadId = cv.GetLongValue("", "DebugReloadId", 0x0);
		group
			->DebugReload = dataHandler->LookupForm<RE::TESGlobal>(debugReloadId, debugReloadFile);

		std::vector<std::shared_ptr<TreeNode>> nodes;
		for (std::int32_t i = 0; i < MaxNodes; i++) {
			auto enable = cv.GetBoolValue("", fmt::format("Node{}.Enable"sv, i).c_str());
			if (!enable) {
				continue;
			}

			auto& tn = nodes.emplace_back(std::make_shared<TreeNode>());
			tn->Index = i;

			auto perkFile = GetStringValue("", fmt::format("Node{}.PerkFile"sv, i).c_str(), "");
			auto perkId = cv.GetLongValue("", fmt::format("Node{}.PerkId"sv, i).c_str(), 0x0);
			tn->Perk = dataHandler->LookupForm<RE::BGSPerk>(perkId, perkFile);

			tn->X = (float)cv.GetDoubleValue("", fmt::format("Node{}.X"sv, i).c_str());
			tn->Y = (float)cv.GetDoubleValue("", fmt::format("Node{}.Y"sv, i).c_str());
			tn->GridX = cv.GetLongValue("", fmt::format("Node{}.GridX"sv, i).c_str());
			tn->GridY = cv.GetLongValue("", fmt::format("Node{}.GridY"sv, i).c_str());

			std::string links = GetStringValue("", fmt::format("Node{}.Links"sv, i).c_str(), "");
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
								"Format error in Node{}.Links! Unable to parse integer from {}!"sv,
								i,
								token));
						return nullptr;
					}
				}
			}
		}

		try {
			sk->Info->perkTree = TreeNode::Create(nodes, sk->Info);
			sk->Info->perkTreeWidth = 3;
		}
		catch (...) {
			Error(
				a_file,
				"Something went wrong when creating skill perk tree! Make sure node 0 exists and "
				"no missing nodes are referenced in links."sv);
		}

		return group;
	}

	void Settings::Error(const std::filesystem::path& a_file, std::string_view a_message)
	{
		logger::error(
			"CustomSkills plugin: Failed to read skill from file `{}`: {}"sv,
			a_file.string(),
			a_message);
	}
}
