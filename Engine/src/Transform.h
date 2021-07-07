#pragma once
#include <DirectXMath.h>

#include "Component.h"

class Transform final : public Component
{
	friend Entity;
public:
	void Reset();

	void TranslateWorld(const DirectX::XMFLOAT3& translation);
	void TranslateWorld(float x, float y, float z);
	void TranslateLocal(const DirectX::XMFLOAT3& translation);
	void TranslateLocal(float x, float y, float z);
	void SetLocalPosition(const DirectX::XMFLOAT3& position);
	void SetLocalPosition(float x, float y, float z);
	const DirectX::XMFLOAT3& GetLocalPosition() const;
	DirectX::XMFLOAT3 GetWorldPosition() const;

	void LocalScale(const DirectX::XMFLOAT3& scale);
	void LocalScale(float x, float y, float z);
	bool isLocalScaleUniform() const;
	void SetLocalScale(float x, float y, float z);
	const DirectX::XMFLOAT3& GetLocalScale() const;
	void SetLocalScale(const DirectX::XMFLOAT3& scale);

	//Angles are in degrees.
	//Rotation order is Z-X-Y.
	//Axis must be normalized.
	void RotateWorld(const DirectX::XMFLOAT3& eulerAngles);
	void RotateWorld(float x, float y, float z);
	void RotateWorld(const DirectX::XMFLOAT3& axis, float angle);
	void RotateLocal(const DirectX::XMFLOAT3& eulerAngles);
	void RotateLocal(float x, float y, float z);
	void RotateLocal(const DirectX::XMFLOAT3& axis, float angle);
	void RotateAround(DirectX::XMFLOAT3 point, DirectX::XMFLOAT3 axis, float angle);
	DirectX::XMVECTOR GetWorldQuaternion() const;
	const DirectX::XMVECTOR& GetLocalQuaternion() const;
	void SetLocalRotation(float x, float y, float z);

	//World Space (relative to it's parent)
	DirectX::XMVECTOR GetUp() const;
	DirectX::XMVECTOR GetRight() const;
	DirectX::XMVECTOR GetForward() const;

	static const DirectX::XMVECTOR& GetWorldUp();
	static const DirectX::XMVECTOR& GetWorldRight();
	static const DirectX::XMVECTOR& GetWorldForward();

	//Relative to it's parent.
	DirectX::XMMATRIX GetLocalMatrix() const;
	void SetLocalMatrix(const DirectX::XMMATRIX& matrix);

	//Transform's world space matrix.
	DirectX::XMMATRIX GetWorldMatrix() const;

	Transform* Clone() override;
private:
	DirectX::XMFLOAT3 localPosition = { 0.0f,0.0f,0.0f }; //Relative to it's parent.
	DirectX::XMVECTOR localQuaternion = DirectX::XMQuaternionIdentity(); //Relative to it's parent.
	DirectX::XMFLOAT3 localScale = { 1.0f,1.0f,1.0f }; //Relative to it's parent.
	mutable DirectX::XMMATRIX localMatrix = DirectX::XMMatrixIdentity();
	mutable bool isLocalMatrixUpdated = true;
};