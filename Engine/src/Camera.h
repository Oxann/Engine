#pragma once
#include "Component.h"

class Camera : public Component
{
public:
	void Start() override;
	Camera* Clone() override;
	void UpdateViewMatrix();
	void SetActive();
private:
};