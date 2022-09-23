#include "Skeleton.h"
#include "Transform.h"
#include "Entity.h"

Skeleton::Skeleton(Transform* root)
{
	boneMatrices.reserve(100);
	Init(root);
	boneMatrices.shrink_to_fit();
}

const DirectX::XMMATRIX& Skeleton::GetBoneMatrix(unsigned int index) const
{
	return boneMatrices[index];
}

void Skeleton::SetBoneMatrix(unsigned int index, const DirectX::XMMATRIX& worldMatrix)
{
	boneMatrices[index] = DirectX::XMMatrixInverse(nullptr, worldMatrix);
}

int Skeleton::GetBoneIndex(std::string_view name)
{
	auto result = boneNames.find(name.data());
	return result == boneNames.end() ? -1 : result->second;
}

void Skeleton::Init(Transform* root)
{
	boneNames.emplace(root->GetEntity()->name, boneMatrices.size());
	boneMatrices.emplace_back(DirectX::XMMatrixInverse(nullptr,root->GetWorldMatrix()));
	
	const auto childrenCount = root->GetEntity()->GetChildrenCount();
	for (int i = 0; i < childrenCount; i++)
		Init(root->GetEntity()->GetChild(i)->GetTransform());
}
