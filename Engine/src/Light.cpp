#include "Light.h"

void DirectionalLight::Start()
{
	if (lights.size() == 0u)
		lights.reserve(4u);
	
	lights.push_back(this);
}

DirectionalLight* DirectionalLight::Clone()
{
	return new DirectionalLight(*this);
}

void PointLight::Start()
{
	if (lights.size() == 0u)
		lights.reserve(4u);
	
	lights.push_back(this);
}

PointLight* PointLight::Clone()
{
	return new PointLight(*this);
}
