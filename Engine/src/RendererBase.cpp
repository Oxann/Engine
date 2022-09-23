#include "RendererBase.h"
#include "Graphics.h"
#include "Entity.h"
#include "Scene.h"


void RendererBase::SetMaterial(Material* material, unsigned int materialIndex)
{
	if (materialIndex >= materials.size())
	{
		std::stringstream ss;
		ss << "Entity name: " << GetEntity()->name << std::endl
			<< "Component: Renderer" << std::endl
			<< "Material index is not correct, it must be greater than zero and less than submeshcount - 1";
		THROW_ENGINE_EXCEPTION(ss.str(), true);
	}
	else
		materials[materialIndex] = material;
}

const std::vector<Material*>& RendererBase::GetMaterials() const
{
	return materials;
}