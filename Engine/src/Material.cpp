#include "Material.h"

void Material::Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const
{
	const auto& vertexElements = shaderView.GetActiveVertexShader().GetVertexElements();

	for (int i = 0; i < vertexElements.size(); i++)
		subMesh->GetVertexElement(vertexElements[i])->BindPipeline();

	
}
