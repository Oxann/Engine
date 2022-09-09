#include "BoxCollider.h"

BoxCollider::BoxCollider()
{
	collider->setCollisionShape(new btBoxShape(btVector3(0.5f, 0.5f, 0.5f)));
}

BoxCollider* BoxCollider::Clone()
{
	return nullptr;
}

void BoxCollider::SetSize(float x, float y, float z)
{
	collider->getCollisionShape()->setLocalScaling(btVector3(x,y,z));
}

DirectX::XMFLOAT3 BoxCollider::GetSize()
{
	btVector3 size = collider->getCollisionShape()->getLocalScaling();
	return { size.x(), size.y() , size.z() };
}
