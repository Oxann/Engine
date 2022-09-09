#include "SphereCollider.h"

SphereCollider::SphereCollider()
{
	collider->setCollisionShape(new btSphereShape(0.5f));
}

SphereCollider* SphereCollider::Clone()
{
    return nullptr;
}

void SphereCollider::SetRadius(float radius)
{
	radius *= 2.0f;
	collider->getCollisionShape()->setLocalScaling(btVector3(radius, radius, radius));
}

float SphereCollider::GetRadius() const
{
	return collider->getCollisionShape()->getLocalScaling().getX() * 0.5f;
}
