#include "Camera.h"
#include "Graphics.h"
#include "Transform.h"
#include "Scene.h"

void Camera::Start()
{
	if (Scene::GetActiveScene()->rendererManager.activeCamera == nullptr)
		SetActive();
}

Camera* Camera::Clone()
{
	return new Camera;
}

void Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 position = GetTransform()->GetWorldPosition();

	Graphics::viewMatrix = DirectX::XMMatrixTranslationFromVector(DirectX::XMVectorNegate(DirectX::XMLoadFloat3(&position))) * 
						   DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationQuaternion(GetTransform()->GetWorldQuaternion()));
}

void Camera::SetActive()
{
	Scene::GetActiveScene()->rendererManager.activeCamera = this;
}
