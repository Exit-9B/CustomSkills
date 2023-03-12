#pragma once

namespace CustomSkills
{
	class TreeNode final
	{
	public:
		static RE::BGSSkillPerkTreeNode* Create(
			const std::vector<std::shared_ptr<TreeNode>>& a_nodes);

	private:
		static void LinkNode(
			RE::BGSSkillPerkTreeNode* a_source,
			RE::BGSSkillPerkTreeNode* a_target);

		static RE::BGSSkillPerkTreeNode* CreateNode(std::int32_t a_index);

	public:
		std::int32_t Index = -1;
		std::vector<std::int32_t> Links;
		std::string PerkFile;
		RE::FormID PerkId = 0x0;
		std::int32_t GridX = 0;
		std::int32_t GridY = 0;
		float X = 0.0f;
		float Y = 0.0f;
	};
}
