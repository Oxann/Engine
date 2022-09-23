#pragma once

#include <DirectXMath.h>
#include <vector>
#include <map>
#include <memory>
#include <optional>

#include "Skeleton.h"

class Animation
{
	friend class Animator;
public:
	struct Key
	{
		DirectX::XMVECTOR value;
		float time;
	};

	struct Node
	{
		std::vector<Key> positions;
		std::vector<Key> rotations;
		std::vector<Key> scales;
	};
public:
	Animation(std::shared_ptr<Skeleton> skeleton, float duration);

	Node* AddNewNodeFromSkeleton(std::string_view nodeName);
	Node* GetNodeKeys(std::string_view nodeName);
	Node* GetNodeKeys(unsigned int nodeIndex);
public:
	std::string name;
private:
	std::shared_ptr<Skeleton> skeleton;
	std::vector<std::pair<unsigned int, Node>> nodes;
	float duration;
};