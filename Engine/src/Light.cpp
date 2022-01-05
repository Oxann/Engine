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

void PointLight::Start()
{
	Scene::GetActiveScene()->rendererManager.pointLights.push_back(this);
}

PointLight* PointLight::Clone()
{
	return new PointLight(*this);
}
