#include "Light.h"
#include "Entity.h"

//Directional
std::vector<const DirectionalLight*> DirectionalLight::lights;

//Point
std::vector<const PointLight*> PointLight::lights;
float PointLight::constant = 1.0f;
float PointLight::linear = 0.14f;
float PointLight::quadratic = 0.07f;

void DirectionalLight::Start()
{
	if (lights.size() == 0u)
		lights.reserve(4u);
	lights.push_back(this);
}

DirectionalLight* DirectionalLight::Clone()
{
	DirectionalLight* newDirectionalLight = new DirectionalLight;
	newDirectionalLight->color = this->color;
	newDirectionalLight->intensity = this->intensity;
	return newDirectionalLight;
}

void PointLight::Start()
{
	if (lights.size() == 0u)
		lights.reserve(4u);
	lights.push_back(this);
}

PointLight* PointLight::Clone()
{
	PointLight* newPointLight = new PointLight;
	newPointLight->color = this->color;
	newPointLight->intensity = this->intensity;
	newPointLight->range = this->range;
	return newPointLight;
}
