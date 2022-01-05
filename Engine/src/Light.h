#pragma once
#include "ConstantBuffer.h"
#include "Component.h"
#include "ShadowMap.h"

class DirectionalLight final : public Component
{
	friend class Renderer;
	friend class RendererManager;
public:
	void Start() override;
	DirectionalLight* Clone() override;
public:
	DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	float depthBias = 0.0f;
	bool shadows = false;
public:
	static constexpr unsigned int MaxCount = 4u;
private:
	ShadowMap shadowMap;
};

class PointLight final : public Component
{
	friend class Renderer;
	friend class RendererManager;
public:
	void Start() override;
	PointLight* Clone() override;
public:
	DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	float range = 5.0f;
public:
	inline static float constant = 1.0f;
	inline static float linear = 0.14f;
	inline static float quadratic = 0.07f;
	inline static constexpr unsigned int MaxCount = 4u;
};