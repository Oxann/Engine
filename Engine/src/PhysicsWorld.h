#pragma once

#include "btBulletDynamicsCommon.h"

class PhysicsWorld
{
	friend class Scene;
	friend class Physics;
public:
	void SetGravity(float x, float y, float z);
	void AddRigidbody(class Rigidbody* body);
	void AddCollider(class Collider* collider);
private:
	PhysicsWorld();
private:
	btDiscreteDynamicsWorld* world;
};