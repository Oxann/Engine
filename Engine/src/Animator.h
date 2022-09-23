#pragma once

#include "Animation.h"
#include "Component.h"

class Animator : public Component
{
	struct LastKeyIndices
	{
		unsigned int positionIndex;
		unsigned int rotationIndex;
		unsigned int scaleIndex;
	};
public:
	void Start() override;
	void Update() override;
	Animator* Clone() override;

	void SetPlayingAnimation(std::shared_ptr<Animation> animation);

	float GetCurrentAnimationTime() const;
	float GetCurrentAnimationDuration() const;
private:
	std::shared_ptr<Animation> currentAnimation;
	float currentAnimationTime = 0.0f;
	class SkinnedRenderer* renderer;
	LastKeyIndices lastKeyIndices[80];
};