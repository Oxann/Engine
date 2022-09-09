#pragma once
#include "Component.h"
#include "ShadowMap.h"
#include <DirectXMath.h>
#include <utility>

class DirectionalLight final : public Component
{
	friend class Renderer;
	friend class RendererManager;
public:
	void Start() override;
	DirectionalLight* Clone() override;
	void SetShadowResolution(int width, int height);
	int GetShadowResolutionWidth() const;
	int GetShadowResolutionHeight() const;
	void SetShadowType(ShadowMap::ShadowType type);
	ShadowMap::ShadowType GetShadowType() const;
	void SetBias(float bias);
	float GetBias();

public:
	DirectX::XMFLOAT3 color = { 1.0f,1.0f,1.0f };
	float intensity = 1.0f;
	bool shadows = false;
	float shadowDistance = 50.0f;
public:
	static constexpr unsigned int MaxCount = 4u;
	inline static std::pair<int,int> defaultShadowMapResolution = std::make_pair<int,int>(4096,4096);
private:
	ShadowMap shadowMap = { defaultShadowMapResolution.first, defaultShadowMapResolution.second, ShadowMap::ShadowType::Hard };
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
	float range = 50.0f;
	inline static constexpr unsigned int MaxCount = 4u;
};