#pragma once

#include <vector>
#include <unordered_map>
#include <DirectXMath.h>

class Skeleton
{
public:
	//Creates a skeleton from a transform hierarchy.
	Skeleton(class Transform* root);
	
	const DirectX::XMMATRIX& GetBoneMatrix(unsigned int index) const;
	void SetBoneMatrix(unsigned int index, const DirectX::XMMATRIX& worldMatrix);

	//If there is no bone with the given name then this function returns -1.
	int GetBoneIndex(std::string_view name);

private:
	void Init(Transform* root);
private:
	std::vector<DirectX::XMMATRIX> boneMatrices;
	std::vector<std::string> names;
	std::unordered_map<std::string, unsigned int> boneNames;
};