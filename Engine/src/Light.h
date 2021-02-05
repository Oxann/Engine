#pragma once
#include "ConstantBuffer.h"
#include "Component.h"

class DirectionalLight final : public Component
{
	friend class Renderer;
public:
	void Start() override;
	DirectionalLight* Clone() override;
public:
	DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	float intensity = 1.0f;
public:
	static constexpr unsigned int MaxCount = 4u;
private:
	static std::vector<const DirectionalLight*> lights;
};

class PointLight final : public Component
{
	friend class Renderer;
public:
	void Start() override;
	PointLight* Clone() override;
public:
	DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	float range = 5.0f;
public:
	static float constant;
	static float linear;
	static float quadratic;
	static constexpr unsigned int MaxCount = 4u;
private:
	static std::vector<const PointLight*> lights;
};