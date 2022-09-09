#include "Light.h"
#include "Scene.h"

void DirectionalLight::Start()
{
	Scene::GetActiveScene()->rendererManager.directionalLights.push_back(this);
}

DirectionalLight* DirectionalLight::Clone()
{
	return new DirectionalLight(*this);
}

void DirectionalLight::SetShadowResolution(int width, int height)
{
	shadowMap.SetResolution(width, height);
}

int DirectionalLight::GetShadowResolutionWidth() const
{
	return shadowMap.GetWidth();
}

int DirectionalLight::GetShadowResolutionHeight() const
{
	return shadowMap.GetHeight();
}

void DirectionalLight::SetShadowType(ShadowMap::ShadowType type)
{
	shadowMap.SetShadowType(type);
}

ShadowMap::ShadowType DirectionalLight::GetShadowType() const
{
	return shadowMap.GetShadowType();
}

void DirectionalLight::SetBias(float bias)
{
	shadowMap.SetBias(bias);
}

float DirectionalLight::GetBias()
{
	return shadowMap.GetBias();
}

void PointLight::Start()
{
	Scene::GetActiveScene()->rendererManager.pointLights.push_back(this);
}

PointLight* PointLight::Clone()
{
	return new PointLight(*this);
}
