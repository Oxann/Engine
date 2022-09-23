#include "Animator.h"
#include "EngineAssert.h"
#include "Entity.h"
#include "SkinnedRenderer.h"
#include "Time.h"
#include "Input.h"

void Animator::Start()
{
	renderer = GetEntity()->GetComponent<SkinnedRenderer>();
	ENGINEASSERT(renderer, "Animator needs a skinned renderer.")
}

void Animator::Update()
{
	auto& bones = renderer->GetBones();

	for (int i = 0; i < currentAnimation->nodes.size(); i++)
	{
		auto& node = currentAnimation->nodes[i];
		const auto& lastKeyIndicesOfThisBone = lastKeyIndices[node.first];

		const auto& lastPositionKey = node.second.positions[lastKeyIndicesOfThisBone.positionIndex];
		const auto& lastRotationKey = node.second.rotations[lastKeyIndicesOfThisBone.rotationIndex];
		const auto& lastScaleKey = node.second.scales[lastKeyIndicesOfThisBone.scaleIndex];
		
		Transform* bone = bones[node.first].transform;

		if (node.second.positions.size() - 1 > lastKeyIndicesOfThisBone.positionIndex)
		{
			const auto& nextPositionKey = node.second.positions[lastKeyIndicesOfThisBone.positionIndex + 1];

			if (currentAnimationTime < lastPositionKey.time)
      				int x = 10;

			float lerpAmount = (currentAnimationTime - lastPositionKey.time) / (nextPositionKey.time - lastPositionKey.time);
			
			DirectX::XMFLOAT3 finalPosition;
			DirectX::XMStoreFloat3(&finalPosition, DirectX::XMVectorLerp(lastPositionKey.value, nextPositionKey.value, lerpAmount));

			bone->SetLocalPosition(finalPosition);
		}
		else
		{
			DirectX::XMFLOAT3 finalPosition;
			DirectX::XMStoreFloat3(&finalPosition, lastPositionKey.value);
			bone->SetLocalPosition(finalPosition);
		}

		if (node.second.rotations.size() - 1 > lastKeyIndicesOfThisBone.rotationIndex)
		{
			const auto& nextRotationKey = node.second.rotations[lastKeyIndicesOfThisBone.rotationIndex + 1];
			float lerpAmount = (currentAnimationTime - lastRotationKey.time) / (nextRotationKey.time - lastRotationKey.time);

			DirectX::XMFLOAT4 finalRotation;
			DirectX::XMStoreFloat4(&finalRotation, DirectX::XMQuaternionSlerp(lastRotationKey.value, nextRotationKey.value, lerpAmount));

			bone->SetLocalRotation(finalRotation.x, finalRotation.y, finalRotation.z, finalRotation.w);
		}
		else
		{
			DirectX::XMFLOAT4 finalRotation;
			DirectX::XMStoreFloat4(&finalRotation, lastRotationKey.value);
			bone->SetLocalRotation(finalRotation.x, finalRotation.y, finalRotation.z, finalRotation.w);
		}
		
		if (node.second.scales.size() - 1 > lastKeyIndicesOfThisBone.scaleIndex)
		{
			const auto& nextScaleKey = node.second.scales[lastKeyIndicesOfThisBone.scaleIndex + 1];
			float lerpAmount = (currentAnimationTime - lastScaleKey.time) / (nextScaleKey.time - lastScaleKey.time);

			DirectX::XMFLOAT3 finalScale;
			DirectX::XMStoreFloat3(&finalScale, DirectX::XMVectorLerp(lastScaleKey.value, nextScaleKey.value, lerpAmount));

			bone->SetLocalScale(finalScale);
		}
		else
		{
			DirectX::XMFLOAT3 finalScale;
			DirectX::XMStoreFloat3(&finalScale, lastScaleKey.value);
			bone->SetLocalScale(finalScale);
		}
	}

	currentAnimationTime += Time::GetDeltaTime();
	
	if (currentAnimationTime >= currentAnimation->duration)
		currentAnimationTime -= currentAnimation->duration;

	for (int i = 0; i < currentAnimation->nodes.size(); i++)
	{
		auto& node = currentAnimation->nodes[i];
		auto& lastKeyIndicesOfThisBone = lastKeyIndices[node.first];

		for (int j = 1; j < node.second.positions.size(); j++)
		{
			if (node.second.positions[j].time >= currentAnimationTime)
			{
				lastKeyIndicesOfThisBone.positionIndex = j - 1;
				break;
			}
		}
		
		for (int j = 1; j < node.second.rotations.size(); j++)
		{
			if (node.second.rotations[j].time >= currentAnimationTime)
			{
				lastKeyIndicesOfThisBone.rotationIndex = j - 1;
				break;
			}
		}
		
		for (int j = 1; j < node.second.scales.size(); j++)
		{
			if (node.second.scales[j].time >= currentAnimationTime)
			{
				lastKeyIndicesOfThisBone.scaleIndex = j - 1;
				break;
			}
		}
	}
}

Animator* Animator::Clone()
{
	return new Animator;
}

void Animator::SetPlayingAnimation(std::shared_ptr<Animation> animation)
{
	currentAnimation = animation;
}

float Animator::GetCurrentAnimationTime() const
{
	return currentAnimationTime;
}

float Animator::GetCurrentAnimationDuration() const
{
	return currentAnimation->duration;
}
