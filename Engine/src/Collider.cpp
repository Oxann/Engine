#include "Collider.h"
#include "Physics.h"
#include "PhysicsWorld.h"
#include "Transform.h"

Collider::Collider()
{
	collider = new btCollisionObject();
	collider->setCollisionFlags(collider->getCollisionFlags() | btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
}

void Collider::Start()
{
	collider->setUserPointer(GetTransform());
	Physics::GetActiveSceneWorld()->AddCollider(this);
}
