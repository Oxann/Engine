#pragma once
#include "Component.h"
#include <DirectXMath.h>

class Camera : public Component
{
	friend class RendererManager;
public:
	enum class ProjectionType
	{
		Orthographic,
		Perspective
	};
public:
	void Start() override;
	Camera* Clone() override;

	void SetActive();
	void SetProjection(ProjectionType type, float aspectRatio, float height, float near_z, float far_z);
	
	//If the projection type is orthographic then this returns less than 0.
	//Angle in degrees.
	float GetVerticalFOV() const;
	
	//If the projection type is orthographic then this returns less than 0.
	//Angle in degrees.
	float GetHorizontalFOV() const;

private:
	void UpdateMatrices();

private:
	float horizontalFOV;
	float verticalFOV;
	ProjectionType projectionType;
	DirectX::XMMATRIX projectionMatrix;
};