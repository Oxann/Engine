#include "Material.h"
#include "Renderer.h"

void Material::Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const
{
	const auto& vertexElements = shaderView.GetActiveVertexShader().GetVertexElements();

	for (int i = 0; i < vertexElements.size(); i++)
		subMesh->GetVertexElement(vertexElements[i])->BindPipeline();

	//Updating vertex shader per object buffer for this renderer.
	Shader::VertexShaderPerObjectBuffer::buffer.model = renderer->GetWorldMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.modelView = renderer->GetWorldViewMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.modelViewProjection = renderer->GetWorldViewProjectionMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.normal = renderer->GetNormalMatrix();
	Shader::GetVertexShaderPerObjectBuffer()->ChangeData(&Shader::VertexShaderPerObjectBuffer::buffer);


}
