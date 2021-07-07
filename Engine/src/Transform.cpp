#include "Entity.h"
#include "Transform.h"
#include <cmath>

void Transform::Reset()
{
	isLocalMatrixUpdated = false;

	localPosition = { 0.0f,0.0f,0.0f };
	localScale = { 1.0f,1.0f,1.0f };
	localQuaternion = DirectX::XMQuaternionIdentity();
}

void Transform::TranslateWorld(const DirectX::XMFLOAT3& translation)
{
	TranslateWorld(translation.x, translation.y, translation.z);
}

void Transform::TranslateWorld(float x, float y, float z)
{
	isLocalMatrixUpdated = false;

	if (GetEntity()->GetParent())
	{
		DirectX::XMVECTOR worldToLocalQuat = DirectX::XMQuaternionConjugate(GetEntity()->GetParent()->GetTransform()->GetWorldQuaternion());

		DirectX::XMVECTOR rightTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldRight(), worldToLocalQuat), x);
		DirectX::XMVECTOR upTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldUp(), worldToLocalQuat), y);
		DirectX::XMVECTOR forwardTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldForward(), worldToLocalQuat), z);

		DirectX::XMVECTOR newlocalPosition = DirectX::XMVectorAdd(rightTranslation, upTranslation);
		newlocalPosition = DirectX::XMVectorAdd(newlocalPosition, forwardTranslation);
		newlocalPosition = DirectX::XMVectorAdd(newlocalPosition, DirectX::XMLoadFloat3(&localPosition));

		DirectX::XMStoreFloat3(&localPosition, newlocalPosition);
	}
	else
	{
		localPosition.x += x;
		localPosition.y += y;
		localPosition.z += z;
	}
}

void Transform::TranslateLocal(const DirectX::XMFLOAT3& translation)
{
	TranslateLocal(translation.x, translation.z, translation.y);
}

void Transform::TranslateLocal(float x, float y, float z)
{
	isLocalMatrixUpdated = false;

	DirectX::XMVECTOR rightTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldRight(), localQuaternion), x);
	DirectX::XMVECTOR upTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldUp(), localQuaternion), y);
	DirectX::XMVECTOR forwardTranslation = DirectX::XMVectorScale(DirectX::XMVector3Rotate(GetWorldForward(), localQuaternion), z);

	DirectX::XMVECTOR newPosition = DirectX::XMVectorAdd(rightTranslation, upTranslation);
	newPosition = DirectX::XMVectorAdd(newPosition, forwardTranslation);
	newPosition = DirectX::XMVectorAdd(newPosition, DirectX::XMLoadFloat3(&localPosition));

	DirectX::XMStoreFloat3(&localPosition, newPosition);
}

void Transform::SetLocalPosition(const DirectX::XMFLOAT3& localPosition)
{
	isLocalMatrixUpdated = false;
	this->localPosition = localPosition;
}

void Transform::SetLocalPosition(float x, float y, float z)
{
	isLocalMatrixUpdated = false;
	localPosition.x = x;
	localPosition.y = y;
	localPosition.z = z;
}

const DirectX::XMFLOAT3& Transform::GetLocalPosition() const
{
	return localPosition;
}

DirectX::XMFLOAT3 Transform::GetWorldPosition() const
{
	if (GetEntity()->GetParent())
	{
		DirectX::XMFLOAT3 worldPos;
		DirectX::XMStoreFloat3(&worldPos, GetWorldMatrix().r[3]);
		return worldPos;
	}
	else
		return localPosition;
}

void Transform::LocalScale(const DirectX::XMFLOAT3& localScale)
{
	isLocalMatrixUpdated = false;
	this->localScale.x *= localScale.x;
	this->localScale.y *= localScale.y;
	this->localScale.z *= localScale.z;
}

void Transform::SetLocalScale(const DirectX::XMFLOAT3& localScale)
{
	isLocalMatrixUpdated = false;
	this->localScale.x = localScale.x;
	this->localScale.y = localScale.y;
	this->localScale.z = localScale.z;
}

void Transform::LocalScale(float x, float y, float z)
{
	LocalScale({ x,y,z });
}

void Transform::SetLocalScale(float x, float y, float z)
{
	SetLocalScale({ x,y,z });
}

const DirectX::XMFLOAT3& Transform::GetLocalScale() const
{
	return localScale;
}

bool Transform::isLocalScaleUniform() const
{
	return localScale.x == localScale.y &&
		localScale.x == localScale.z &&
		localScale.z == localScale.y;
}

void Transform::RotateWorld(const DirectX::XMFLOAT3& eulerAngles)
{
	RotateWorld(eulerAngles.x, eulerAngles.y, eulerAngles.z);
}

void Transform::RotateWorld(float x, float y, float z)
{
	isLocalMatrixUpdated = false;
	DirectX::XMVECTOR rotationQuat;

	if (GetEntity()->GetParent())
	{
		DirectX::XMVECTOR worldToLocalQuat = DirectX::XMQuaternionConjugate(GetEntity()->GetParent()->GetTransform()->GetWorldQuaternion());
		DirectX::XMVECTOR worldUp = DirectX::XMVector3Rotate(GetWorldUp(), worldToLocalQuat);
		DirectX::XMVECTOR worldRight = DirectX::XMVector3Rotate(GetWorldRight(), worldToLocalQuat);
		DirectX::XMVECTOR worldForward = DirectX::XMVector3Rotate(GetWorldForward(), worldToLocalQuat);

		rotationQuat = DirectX::XMQuaternionMultiply(DirectX::XMQuaternionRotationNormal(worldForward, DirectX::XMConvertToRadians(z)),
			DirectX::XMQuaternionRotationNormal(worldRight, DirectX::XMConvertToRadians(x)));
		rotationQuat = DirectX::XMQuaternionMultiply(rotationQuat,
			DirectX::XMQuaternionRotationNormal(worldUp, DirectX::XMConvertToRadians(y)));
	}
	else
	{
		rotationQuat = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(x), DirectX::XMConvertToRadians(y), DirectX::XMConvertToRadians(z));
	}

	localQuaternion = DirectX::XMQuaternionMultiply(localQuaternion, rotationQuat);
}

void Transform::RotateWorld(const DirectX::XMFLOAT3& axis, float angle)
{
	isLocalMatrixUpdated = false;
	DirectX::XMVECTOR axis_vector = DirectX::XMLoadFloat3(&axis);

	if (GetEntity()->GetParent())
	{
		DirectX::XMVECTOR worldToLocalQuat = DirectX::XMQuaternionConjugate(GetEntity()->GetParent()->GetTransform()->GetWorldQuaternion());
		DirectX::XMVector3Rotate(axis_vector, worldToLocalQuat);
	}

	//Q1 is orientation because we want to apply rotationQuat in World Space.
	localQuaternion = DirectX::XMQuaternionMultiply(
		localQuaternion,
		DirectX::XMQuaternionRotationNormal(axis_vector, DirectX::XMConvertToRadians(angle)));
}

void Transform::RotateLocal(const DirectX::XMFLOAT3& eulerAngles)
{
	RotateLocal(eulerAngles.x, eulerAngles.y, eulerAngles.z);
}

void Transform::RotateLocal(float x, float y, float z)
{
	RotateLocal({ 0.0f,0.0f,1.0f }, z);
	RotateLocal({ 1.0f,0.0f,0.0f }, x);
	RotateLocal({ 0.0f,1.0f,0.0f }, y);
}

void Transform::RotateLocal(const DirectX::XMFLOAT3& axis, float angle)
{
	isLocalMatrixUpdated = false;

	//Q1 is rotationQuat because we want to apply rotationQuat in Local Space.
	localQuaternion = DirectX::XMQuaternionMultiply(
		DirectX::XMQuaternionRotationNormal(DirectX::XMLoadFloat3(&axis), DirectX::XMConvertToRadians(angle)),
		localQuaternion);
}

void Transform::RotateAround(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 axis, float angle)
{	
	isLocalMatrixUpdated = false;
	DirectX::XMVECTOR axis_vector = DirectX::XMLoadFloat3(&axis);
	DirectX::XMVECTOR point_vector = DirectX::XMLoadFloat3(&point);

	if (GetEntity()->GetParent())
	{
		DirectX::XMVECTOR worldToLocalQuat = DirectX::XMQuaternionConjugate(GetEntity()->GetParent()->GetTransform()->GetWorldQuaternion());
		DirectX::XMVector3Rotate(axis_vector, worldToLocalQuat);

		DirectX::XMMATRIX worldToLocalMatrix = DirectX::XMMatrixInverse(nullptr, GetEntity()->GetParent()->GetTransform()->GetWorldMatrix());
		point_vector = DirectX::XMVector3TransformCoord(point_vector, worldToLocalMatrix);
	}

	//Rotating object
	localQuaternion = DirectX::XMQuaternionMultiply(localQuaternion,
		DirectX::XMQuaternionRotationNormal(axis_vector, DirectX::XMConvertToRadians(angle)));
	
	//Moving object
	DirectX::XMVECTOR dif = DirectX::XMVectorSubtract(DirectX::XMLoadFloat3(&localPosition), point_vector);
	DirectX::XMStoreFloat3(&localPosition,
		DirectX::XMVectorAdd(point_vector, DirectX::XMVector3Rotate(dif,
			DirectX::XMQuaternionRotationNormal(axis_vector, DirectX::XMConvertToRadians(angle)))));
}

DirectX::XMVECTOR Transform::GetWorldQuaternion() const
{
	if (GetEntity()->GetParent())
		return DirectX::XMQuaternionMultiply(GetLocalQuaternion(), GetEntity()->GetParent()->GetTransform()->GetWorldQuaternion());
	else
		return GetLocalQuaternion();
}

const DirectX::XMVECTOR& Transform::GetLocalQuaternion() const
{
	return localQuaternion;
}

void Transform::SetLocalRotation(float x, float y, float z)
{
	isLocalMatrixUpdated = false;
	localQuaternion = DirectX::XMQuaternionRotationRollPitchYaw(DirectX::XMConvertToRadians(x), DirectX::XMConvertToRadians(y), DirectX::XMConvertToRadians(z));
}

DirectX::XMMATRIX Transform::GetLocalMatrix() const
{
	if (!isLocalMatrixUpdated)
	{
		localMatrix = DirectX::XMMatrixScalingFromVector(DirectX::XMLoadFloat3(&localScale)) *
			DirectX::XMMatrixRotationQuaternion(localQuaternion) *
			DirectX::XMMatrixTranslationFromVector(DirectX::XMLoadFloat3(&localPosition));

		isLocalMatrixUpdated = true;
	}
	return localMatrix;
}

void Transform::SetLocalMatrix(const DirectX::XMMATRIX& matrix)
{	
	isLocalMatrixUpdated = false;

	localMatrix = matrix;

	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR scale;
	DirectX::XMMatrixDecompose(&scale, &localQuaternion, &pos, matrix);
	
	DirectX::XMStoreFloat3(&localPosition, pos);
	DirectX::XMStoreFloat3(&localScale, scale);
}

DirectX::XMMATRIX Transform::GetWorldMatrix() const
{
	auto parent = GetEntity()->GetParent();
	if (parent)
		return GetLocalMatrix() * parent->GetTransform()->GetWorldMatrix();
	else
		return GetLocalMatrix();
}

Transform* Transform::Clone()
{
	Transform* newTransform = new Transform;
	newTransform->localMatrix = this->localMatrix;
	newTransform->localQuaternion = this->localQuaternion;
	newTransform->localScale = this->localScale;
	newTransform->isLocalMatrixUpdated = this->isLocalMatrixUpdated;
	newTransform->localPosition = this->localPosition;
	return newTransform;
}

DirectX::XMVECTOR Transform::GetUp() const
{
	return DirectX::XMVector3Rotate(GetWorldUp(), GetWorldQuaternion());
}

DirectX::XMVECTOR Transform::GetRight() const
{
	return DirectX::XMVector3Rotate(GetWorldRight(), GetWorldQuaternion());
}

DirectX::XMVECTOR Transform::GetForward() const
{
	return DirectX::XMVector3Rotate(GetWorldForward(), GetWorldQuaternion());
}

const DirectX::XMVECTOR& Transform::GetWorldUp()
{
	static DirectX::XMVECTOR WorldUp = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
	return WorldUp;
}

const DirectX::XMVECTOR& Transform::GetWorldRight()
{
	static DirectX::XMVECTOR WorldRight = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 1.0f);
	return WorldRight;
}

const DirectX::XMVECTOR& Transform::GetWorldForward()
{
	static DirectX::XMVECTOR WorldForward = DirectX::XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
	return WorldForward;
}