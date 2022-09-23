#include "Animation.h"
#include "EngineAssert.h"
#include <algorithm>

Animation::Animation(std::shared_ptr<Skeleton> skeleton, float duration)
	:skeleton(skeleton),
	duration(duration)
{
}

Animation::Node* Animation::AddNewNodeFromSkeleton(std::string_view nodeName)
{
	int nodeIndex = skeleton->GetBoneIndex(nodeName);
	
	if (nodeIndex == -1)
		return nullptr;
	else
	{
		nodes.push_back({ nodeIndex, {} });
		return &nodes.back().second;
	}
}

Animation::Node* Animation::GetNodeKeys(std::string_view nodeName)
{
	int nodeIndex = skeleton->GetBoneIndex(nodeName);
	ENGINEASSERT(nodeIndex != -1, "Wrong bone name.");
	GetNodeKeys(nodeIndex);
}

Animation::Node* Animation::GetNodeKeys(unsigned int nodeIndex)
{
	auto result = std::ranges::find_if(nodes, [nodeIndex](const std::pair<unsigned int, Node>& node) {
		return node.first == nodeIndex;
	});
	
	return result == nodes.end() ? nullptr : &result->second;
}
