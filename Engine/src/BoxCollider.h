#pragma once

#include "Collider.h"
#include <DirectXMath.h>

class BoxCollider : public Collider
{
public:
	BoxCollider();
	BoxCollider* Clone() override;
	void SetSize(float x, float y, float z);
	DirectX::XMFLOAT3 GetSize();
};