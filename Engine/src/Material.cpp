#include "Material.h"
#include "Renderer.h"
#include "Graphics.h"
#include <algorithm>

Material::Material(const std::string& name, const Shader* shader)
	:ResourceBase(name),
	shaderView(shader)
{
	materialBuffer.resize(shaderView.shader->GetMaterialBufferSize());
	
	shaderView.ActivateMacro_PS("SHADOW");
	shaderView.ActivateMacro_VS("SHADOW");
}

void Material::ActivateMacro(std::string_view name)
{
	bool isMacroFound = shaderView.ActivateMacro_PS(name);
	
	if (isMacroFound)
		shaderView.ActivateMacro_VS(name);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no MACRO named " + name.data());
}

void Material::DeactivateMacro(std::string_view name)
{
	bool isMacroFound = shaderView.DeactivateMacro_PS(name);

	if (isMacroFound)
		shaderView.DeactivateMacro_VS(name);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no MACRO named " + name.data());
}

void Material::SetFloat(std::string_view name, float value)
{
	auto element = std::find(shaderView.shader->float1s.begin(), shaderView.shader->float1s.end(), name);

	if (element != shaderView.shader->float1s.end())
		memcpy(materialBuffer.data() + element->offset, &value, 4u);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT parameter named " + name.data());
}

void Material::SetFloat2(std::string_view name, DirectX::XMFLOAT2 value)
{
	auto element = std::find(shaderView.shader->float2s.begin(), shaderView.shader->float2s.end(), name);

	if (element != shaderView.shader->float2s.end())
		memcpy(materialBuffer.data() + element->offset, &value, 8u);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT2 parameter named " + name.data());
}

void Material::SetFloat3(std::string_view name, DirectX::XMFLOAT3 value)
{
	auto element = std::find(shaderView.shader->float3s.begin(), shaderView.shader->float3s.end(), name);

	if (element != shaderView.shader->float3s.end())
		memcpy(materialBuffer.data() + element->offset, &value, 12u);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT3 parameter named " + name.data());
}

void Material::SetFloat4(std::string_view name, DirectX::XMFLOAT4 value)
{
	auto element = std::find(shaderView.shader->float4s.begin(), shaderView.shader->float4s.end(), name);

	if (element != shaderView.shader->float4s.end())
		memcpy(materialBuffer.data() + element->offset, &value, 16u);
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT4 parameter named " + name.data());
}

float Material::GetFloat(std::string_view name) const
{
	auto element = std::find(shaderView.shader->float1s.begin(), shaderView.shader->float1s.end(), name);

	float value = 0.0f;

	if (element != shaderView.shader->float1s.end())
	{
		memcpy(&value, materialBuffer.data() + element->offset, 4u);
		return value;
	}
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT parameter named " + name.data());
}

DirectX::XMFLOAT2 Material::GetFloat2(std::string_view name) const
{
	auto element = std::find(shaderView.shader->float2s.begin(), shaderView.shader->float2s.end(), name);

	DirectX::XMFLOAT2 value;

	if (element != shaderView.shader->float2s.end())
	{
		memcpy(&value, materialBuffer.data() + element->offset, 8u);
		return value;
	}
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT2 parameter named " + name.data());
}

DirectX::XMFLOAT3 Material::GetFloat3(std::string_view name) const
{
	auto element = std::find(shaderView.shader->float3s.begin(), shaderView.shader->float3s.end(), name);

	DirectX::XMFLOAT3 value;

	if (element != shaderView.shader->float3s.end())
	{
		memcpy(&value, materialBuffer.data() + element->offset, 12u);
		return value;
	}
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT3 parameter named " + name.data());
}

DirectX::XMFLOAT4 Material::GetFloat4(std::string_view name) const
{
	auto element = std::find(shaderView.shader->float4s.begin(), shaderView.shader->float4s.end(), name);

	DirectX::XMFLOAT4 value;

	if (element != shaderView.shader->float4s.end())
	{
		memcpy(&value, materialBuffer.data() + element->offset, 16u);
		return value;
	}
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no FLOAT4 parameter named " + name.data());
}

void Material::SetTexture(std::string_view name, Texture* texture)
{
	const auto& texture2DDefs = shaderView.GetActivePixelShader().GetTexture2Ds();
	auto textureDef = std::find(texture2DDefs.begin(), texture2DDefs.end(), name);

	if (textureDef != texture2DDefs.end())
		texture2Ds[textureDef->slot] = texture;
	else
		ENGINEASSERT(false, shaderView.shader->name + " has no Texture2D parameter named " + name.data());
}

Texture* Material::GetTexture(std::string_view name) const
{
	const auto& texture2DDefs = shaderView.GetActivePixelShader().GetTexture2Ds();
	auto textureDef = std::find(texture2DDefs.begin(), texture2DDefs.end(), name);

	if (textureDef != texture2DDefs.end())
		return texture2Ds[textureDef->slot];
	else
		return nullptr;
}

unsigned int Material::GetTextureCount() const
{
	return texture2Ds.size();
}

void Material::Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const
{
	shaderView.GetActiveVertexShader().Bind();
	shaderView.GetActivePixelShader().Bind();

	subMesh->GetIndexBuffer()->BindPipeline();
	const auto& vertexElements = shaderView.GetActiveVertexShader().GetVertexElements();

	for (int i = 0; i < vertexElements.size(); i++)
		subMesh->GetVertexElement(vertexElements[i])->BindPipeline();

	//Updating vertex shader per object buffer for this renderer.
	Shader::VertexShaderPerObjectBuffer::buffer.model = renderer->GetWorldMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.modelView = renderer->GetWorldViewMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.modelViewProjection = renderer->GetWorldViewProjectionMatrix();
	Shader::VertexShaderPerObjectBuffer::buffer.normal = renderer->GetNormalMatrix();
	Shader::GetVertexShaderPerObjectBuffer()->ChangeData(&Shader::VertexShaderPerObjectBuffer::buffer);

	shaderView.shader->materialBuffer->BindPipeline();
	shaderView.shader->materialBuffer->ChangeData(materialBuffer.data());

	//Binding textures (also binds sampler)
	const auto& texture2DDefs = shaderView.GetActivePixelShader().GetTexture2Ds();
	for (const auto& texture2DDef : texture2DDefs)
	{
		if(texture2Ds[texture2DDef.slot])
			texture2Ds[texture2DDef.slot]->BindPipeline(texture2DDef.slot);
		else
		{
			static Texture* whiteTexture = Resources::FindTexture("$Default\\White");
			whiteTexture->BindPipeline(texture2DDef.slot);
		}
	}

	if (shaderView.GetActiveVertexShader().HasShadows())
	{
		Shader::VertexShaderShadowBuffer::buffer.lightSpaceMatrix = renderer->lightSpaceMatrix;
		Shader::GetVertexShaderShadowBuffer()->ChangeData(&Shader::VertexShaderShadowBuffer::buffer);
	}
}