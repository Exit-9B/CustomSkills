#include "TreeNode.h"

namespace CustomSkills
{
	RE::BGSSkillPerkTreeNode* TreeNode::Create(
		const std::vector<std::shared_ptr<TreeNode>>& a_nodes,
		RE::ActorValueInfo* a_avInfo)
	{
		std::map<std::int32_t, std::shared_ptr<TreeNode>> itoNode;
		std::map<std::int32_t, RE::BGSSkillPerkTreeNode*> itoMem;

		for (const auto& n : a_nodes) {
			auto ix = n->Index;
			itoNode.emplace(ix, n);
			itoMem.emplace(ix, new RE::BGSSkillPerkTreeNode(ix, a_avInfo));
		}

		for (const auto& n : a_nodes) {
			auto mem = itoMem[n->Index];

			mem->perk = n->Perk;
			mem->unk48 = 1;  // FNAM, unknown
			mem->perkGridX = n->GridX;
			mem->perkGridY = n->GridY;
			mem->horizontalPosition = n->X;
			mem->verticalPosition = n->Y;

			for (const auto& ix : n->Links) {
				LinkNode(mem, itoMem.at(ix));
			}
		}

		return itoMem.at(0);
	}

	void TreeNode::LinkNode(RE::BGSSkillPerkTreeNode* a_source, RE::BGSSkillPerkTreeNode* a_target)
	{
		a_source->children.push_back(a_target);
		a_target->parents.push_back(a_source);
	}
}
