#pragma once
#include <memory>

#include "Component.h"
#include "btBulletCollisionCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

class Collider : public Component
{
	friend class Physics;
	friend class PhysicsWorld;
public:
	Collider();
	void Start();

protected:
	btCollisionObject* collider;
};