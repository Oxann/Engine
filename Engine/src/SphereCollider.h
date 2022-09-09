#pragma once

#include "Collider.h"

class SphereCollider : public Collider
{
public:
	SphereCollider();
	SphereCollider* Clone() override;
	void SetRadius(float radius);
	float GetRadius() const;
};