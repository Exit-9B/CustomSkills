#include "TreeNode.h"

namespace CustomSkills
{
	RE::BGSSkillPerkTreeNode* TreeNode::Create(
		const std::vector<std::shared_ptr<TreeNode>>& a_nodes)
	{
		auto itoNode = std::map<std::int32_t, std::shared_ptr<TreeNode>>();
		auto itoMem = std::map<std::int32_t, RE::BGSSkillPerkTreeNode*>();

		for (auto& n : a_nodes) {
			auto ix = n->Index;
			itoNode.emplace(ix, n);
			itoMem.emplace(ix, CreateNode(ix));
		}

		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			return nullptr;
		}

		for (auto& n : a_nodes) {
			auto mem = itoMem[n->Index];

			if (auto perk = dataHandler->LookupForm<RE::BGSPerk>(n->PerkId, n->PerkFile)) {
				mem->perk = perk;
			}

			mem->unk48 = 1;  // FNAM, unknown
			mem->perkGridX = n->GridX;
			mem->perkGridY = n->GridY;
			mem->horizontalPosition = n->X;
			mem->verticalPosition = n->Y;

			for (auto& ix : n->Links) {
				LinkNode(mem, itoMem[ix]);
			}
		}

		return itoMem[0];
	}

	void TreeNode::LinkNode(RE::BGSSkillPerkTreeNode* a_source, RE::BGSSkillPerkTreeNode* a_target)
	{
		a_source->children.push_back(a_target);
		a_target->parents.push_back(a_source);
	}

	RE::BGSSkillPerkTreeNode* TreeNode::CreateNode(std::int32_t a_index)
	{
		auto avInfo = RE::TESForm::LookupByID<RE::ActorValueInfo>(0x45D);
		if (!avInfo) {
			return nullptr;
		}

		return new RE::BGSSkillPerkTreeNode(a_index, avInfo);
	}
}
