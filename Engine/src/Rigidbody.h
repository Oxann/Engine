#pragma once

#include "Component.h"

class Rigidbody : public Component
{
	friend class PhysicsWorld;
public:
	void Start() override;
	Rigidbody* Clone() override;
	void SetMass(float mass);
	float GetMass() const;
private:
	class btRigidBody* body;
	class btDefaultMotionState* motionState;
};