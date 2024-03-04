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

		auto dir = std::filesystem::path("Data/SKSE/Plugins/CustomSkills");
		std::error_code ec;
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			auto filename = entry.path().filename();

			if (::_wcsicmp(filename.extension().c_str(), L".json") != 0) {
				continue;
			}

			std::string key = filename.stem().string();

			if (const auto sk = ReadSkill(entry.path())) {
				if (::_stricmp(key.data(), "SKILLS") == 0) {
					REL::Relocation<std::uint32_t*> lastSelectedTree{
						RE::Offset::StatsMenu::LastSelectedTree
					};

					if (*lastSelectedTree < sk->Skills.size()) {
						sk->LastSelectedTree = *lastSelectedTree;
					}
				}

				skills.emplace(std::move(key), std::move(sk));
			}
		}

		if (ec) {
			logger::error("Error reading skill configs: {}", ec.message());
		}

		dir = std::filesystem::path("Data/NetScriptFramework/Plugins");
		ec = {};
		for (const auto& entry : std::filesystem::directory_iterator(dir, ec)) {
			if (!entry.is_regular_file()) {
				continue;
			}

			auto filename = entry.path().filename();

			if (::_wcsnicmp(filename.c_str(), L"CustomSkill.", 12) != 0) {
				continue;
			}

			if (::_wcsicmp(filename.extension().c_str(), L".txt") != 0 ||
				::_wcsicmp(filename.stem().extension().c_str(), L".config") != 0) {
				continue;
			}

			std::string key = filename.stem().stem().string().substr(12);

			if (key.empty())
				continue;

			if (const auto sk = ReadLegacySkill(entry.path())) {
				skills.emplace(std::move(key), std::move(sk));
			}
		}

		if (ec) {
			logger::error("Error reading legacy skill configs: {}", ec.message());
		}

		return skills;
	}

	static RE::BSResourceNiBinaryStream& operator>>(
		RE::BSResourceNiBinaryStream& a_sin,
		auto& a_root)
	{
		Json::CharReaderBuilder fact;
		std::unique_ptr<Json::CharReader> const reader{ fact.newCharReader() };

		const auto size = a_sin.stream->totalSize;
		const auto buffer = std::make_unique<char[]>(size);
		a_sin.read(buffer.get(), size);

		const auto begin = buffer.get();
		const auto end = begin + size;

		std::string errs;
		const bool ok = reader->parse(begin, end, &a_root, &errs);

		if (!ok) {
			throw std::runtime_error(errs);
		}

		return a_sin;
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

	static RE::ActorValue ParseSkill(const Json::Value& a_val)
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

		const Json::String str = a_val.asString();
		const auto it = std::ranges::lower_bound(
			SKILLS,
			std::string_view(str),
			util::iless{},
			[](auto&& kv)
			{
				return kv.first;
			});

		if (it != std::end(SKILLS) && ::_stricmp(it->first.data(), str.data()) == 0) {
			return it->second;
		}

		return RE::ActorValue::kHealth;
	}

	auto Settings::ReadSkill(const std::filesystem::path& a_file) -> std::shared_ptr<SkillGroup>
	{
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler)
			return nullptr;

		const auto factory = RE::IFormFactory::GetConcreteFormFactoryByType<RE::ActorValueInfo>();
		if (!factory)
			return nullptr;

		RE::BSResourceNiBinaryStream fileStream{ a_file.string() };

		if (!fileStream.good())
			return nullptr;

		Json::Value root;
		try {
			fileStream >> root;
		}
		catch (...) {
			logger::error("Parse errors in file: {}", a_file.filename().string());
		}

		if (!root.isObject())
			return nullptr;

		auto&& group = std::make_shared<SkillGroup>();

		if (const auto& skydome = root["skydome"]; skydome.isObject()) {
			group->Skydome = skydome["model"].asString();
			if (const auto& cameraRightPoint = skydome["cameraRightPoint"];
				cameraRightPoint.isUInt()) {
				group->CameraRightPoint = cameraRightPoint.asUInt();
			}
		}

		if (group->Skydome.empty()) {
			group->Skydome = "DLC01/Interface/INTVampirePerkSkydome.nif";
		}

		if (const auto& showMenu = root["showMenu"]; showMenu.isString()) {
			group->OpenMenu = ParseForm<RE::TESGlobal>(dataHandler, showMenu);
		}

		if (const auto& debugReload = root["debugReload"]; debugReload.isString()) {
			group->DebugReload = ParseForm<RE::TESGlobal>(dataHandler, debugReload);
		}

		if (const auto& perkPoints = root["perkPoints"]; perkPoints.isString()) {
			group->PerkPoints = ParseForm<RE::TESGlobal>(dataHandler, perkPoints);
		}

		if (const auto& skills = root["skills"]; skills.isArray()) {
			auto actorValue = util::to_underlying(RE::ActorValue::kTotal);
			for (const auto& skill : skills) {
				if (skill.isString()) {
					group->Skills.emplace_back(nullptr);
					group->ActorValues.push_back(ParseSkill(skill.asString()));
					++actorValue;
					continue;
				}
				else if (!skill.isObject()) {
					continue;
				}

				const auto& sk = group->Skills.emplace_back(std::make_shared<Skill>());
				sk->Info = factory->Create();

				group->ActorValues.push_back(static_cast<RE::ActorValue>(actorValue++));

				if (const auto& id = skill["id"]; id.isString()) {
					sk->ID = id.asString();
				}

				if (const auto& name = skill["name"]; name.isString()) {
					std::string tr = name.asString();
					SKSE::Translation::Translate(tr, tr);
					sk->Info->fullName = tr;
				}

				if (const auto& description = skill["description"]; description.isString()) {
					std::string tr = description.asString();
					SKSE::Translation::Translate(tr, tr);
					sk->Description = std::move(tr);
				}

				if (const auto& level = skill["level"]; level.isString()) {
					sk->Level = ParseForm<RE::TESGlobal>(dataHandler, level);
				}

				if (const auto& ratio = skill["ratio"]; ratio.isString()) {
					sk->Ratio = ParseForm<RE::TESGlobal>(dataHandler, ratio);
				}

				if (const auto& legendary = skill["legendary"]; legendary.isString()) {
					sk->Legendary = ParseForm<RE::TESGlobal>(dataHandler, legendary);
				}

				if (const auto& color = skill["color"]; color.isString()) {
					sk->Color = ParseForm<RE::TESGlobal>(dataHandler, color);
				}

				if (const auto& showLevelup = skill["showLevelup"]; showLevelup.isString()) {
					sk->ShowLevelup = ParseForm<RE::TESGlobal>(dataHandler, showLevelup);
				}

				std::vector<std::shared_ptr<TreeNode>> tns;
				if (const auto& nodes = skill["nodes"]; nodes.isArray()) {
					std::map<std::string, std::int32_t> ids;

					const std::int32_t size = (std::min)(128U, nodes.size());
					for (std::int32_t i = 0; i < size; ++i) {
						const auto& node = nodes[i];
						if (const auto& id = node["id"]; id.isString()) {
							ids.emplace(id.asString(), i);
						}
					}

					std::int32_t gridWidth = 1;
					for (const auto& node : nodes) {
						const auto x = node["x"].asDouble();
						gridWidth = (std::max)(
							gridWidth,
							static_cast<std::int32_t>(std::ceil(x * 2)));
					}

					sk->Info->perkTreeWidth = gridWidth;
					const std::uint32_t xMax = (gridWidth + 1) / 2;

					for (std::int32_t i = 0; i < size; ++i) {
						const auto& node = nodes[i];
						const auto& tn = tns.emplace_back(std::make_shared<TreeNode>());
						tn->Index = i;
						if (const auto& perk = node["perk"]; perk.isString()) {
							tn->Perk = ParseForm<RE::BGSPerk>(dataHandler, perk);
						}

						const auto x = node["x"].asDouble() + gridWidth * 0.5;
						const auto y = node["y"].asDouble();
						tn->GridX = (std::max)(static_cast<std::int32_t>(x + 0.58), 0);
						tn->X = static_cast<float>(x - tn->GridX);
						tn->GridY = (std::min)(
							(std::max)(static_cast<std::int32_t>(y - 0.12 * (xMax + 1)), 0),
							4);
						tn->Y = static_cast<float>(y - tn->GridY);

						if (const auto& links = node["links"]; links.isArray()) {
							for (const auto& link : links) {
								if (link.isInt()) {
									tn->Links.push_back(link.asInt());
								}
								else if (link.isString()) {
									tn->Links.push_back(ids[link.asString()]);
								}
							}
						}
					}
				}

				try {
					sk->Info->perkTree = TreeNode::Create(tns, sk->Info);
				}
				catch (...) {
					Error(
						a_file,
						"Something went wrong when creating skill perk tree! Make sure node 0 "
						"exists and no missing nodes are referenced in links.");
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
		auto vampireTree = dataHandler->LookupForm<RE::ActorValueInfo>(0x646, "Skyrim.esm");
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
			auto enable = cv.GetBoolValue("", fmt::format("Node{}.Enable", i).c_str());
			if (!enable) {
				continue;
			}

			auto& tn = nodes.emplace_back(std::make_shared<TreeNode>());
			tn->Index = i;

			auto perkFile = GetStringValue("", fmt::format("Node{}.PerkFile", i).c_str(), "");
			auto perkId = cv.GetLongValue("", fmt::format("Node{}.PerkId", i).c_str(), 0x0);
			tn->Perk = dataHandler->LookupForm<RE::BGSPerk>(perkId, perkFile);

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
		}

		try {
			sk->Info->perkTree = TreeNode::Create(nodes, sk->Info);
			sk->Info->perkTreeWidth = 3;
		}
		catch (...) {
			Error(
				a_file,
				"Something went wrong when creating skill perk tree! Make sure node 0 exists and "
				"no missing nodes are referenced in links.");
		}

		return group;
	}

	void Settings::Error(const std::filesystem::path& a_file, std::string_view a_message)
	{
		logger::error(
			"CustomSkills plugin: Failed to read skill from file `{}`: {}",
			a_file.string(),
			a_message);
	}
}
