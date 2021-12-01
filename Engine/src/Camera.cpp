#include "Camera.h"
#include "Graphics.h"
#include "Transform.h"
#include "Scene.h"
#include "MainWindow.h"

void Camera::Start()
{
	if (Scene::GetActiveScene()->rendererManager.activeCamera == nullptr)
		SetActive();

	SetProjection(ProjectionType::Perspective, (float)MainWindow::GetDisplayResolution().width / (float)MainWindow::GetDisplayResolution().height, 60.0f, 0.05f, 10000.0f);
}

Camera* Camera::Clone()
{
	return new Camera;
}

void Camera::UpdateMatrices()
{
	DirectX::XMFLOAT3 position = GetTransform()->GetWorldPosition();

	Graphics::viewMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&position))) * 
						   DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(GetTransform()->GetWorldQuaternion()));

	Graphics::projectionMatrix = projectionMatrix;
}

void Camera::SetActive()
{
	Scene::GetActiveScene()->rendererManager.activeCamera = this;
}

void Camera::SetProjection(ProjectionType type, float aspectRatio, float height, float near_z, float far_z)
{
	projectionType = type;
	switch (projectionType)
	{
	case Camera::ProjectionType::Orthographic:
		aspectRatio = aspectRatio;
		projectionMatrix = DirectX::XMMatrixOrthographicLH(height * aspectRatio, height, near_z, far_z);
		break;
	case Camera::ProjectionType::Perspective:
		Graphics::aspectRatio = aspectRatio;
		verticalFOV = height;
		horizontalFOV = DirectX::XMConvertToDegrees(2.0f * std::atan(aspectRatio * std::tan(DirectX::XMConvertToRadians(height / 2.0f))));
		projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(verticalFOV), aspectRatio, near_z, far_z);
		break;
	default:
		ENGINEASSERT(false, "Invalid ProjectionType");
		break;
	}
}

float Camera::GetVerticalFOV() const
{
	return projectionType == ProjectionType::Perspective ? verticalFOV : -1.0f;
}

float Camera::GetHorizontalFOV() const
{
	return projectionType == ProjectionType::Perspective ? horizontalFOV : -1.0f;
}
