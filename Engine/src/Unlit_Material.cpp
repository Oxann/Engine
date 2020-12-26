#include "Unlit_Material.h"
#include "Resources.h"


Unlit_Material::Unlit_Material(std::string name)
	:Material(name)
{
	vertexShader = Resources::FindVertexShader("Unlit.cso");
	pixelShader = Resources::FindPixelShader("Unlit.cso");
}

void Unlit_Material::Bind(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	(this->*(Bind_))(subMesh, renderer);
}

void Unlit_Material::SetColor(const DirectX::XMFLOAT4& color)
{
	this->color = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f),
		std::clamp(color.w, 0.0f, 1.0f)
	};
}

void Unlit_Material::SetTexture(const Texture* texture)
{
	this->texture = texture;

	if (texture)
	{
		Bind_ = &Unlit_Material::BindUnlit_T;
		vertexShader = Resources::FindVertexShader("Unlit_T.cso");
		pixelShader = Resources::FindPixelShader("Unlit_T.cso");
	}	
	else
	{
		Bind_ = &Unlit_Material::BindUnlit;
		vertexShader = Resources::FindVertexShader("Unlit.cso");
		pixelShader = Resources::FindPixelShader("Unlit.cso");
	}
}

const Texture* Unlit_Material::GetTexture() const
{
	return texture;
}

bool Unlit_Material::HasTexture() const
{
	return texture;
}

const Material* Unlit_Material::GetDefaultMaterial()
{
	static Unlit_Material defaultMat("UnlitDefaultMat");
	return &defaultMat;
}

void Unlit_Material::BindUnlit(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	//Binding shaders
	vertexShader->BindPipeline();
	pixelShader->BindPipeline();

	//Binding mesh
	subMesh->GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	subMesh->GetIndexBuffer()->BindPipeline();

	//Biding transform
	renderer->UpdateTransformBuffer_Only_MVP();

	//Binding material properties
	struct Material_TO_GPU
	{
		alignas(16u)DirectX::XMFLOAT4 color;
	};
	static Material_TO_GPU toGPU = {};
	toGPU.color = color;

	static PS_ConstantBuffer<Material_TO_GPU> buffer = {
		&toGPU,
		1u,
		PS_MaterialSlot,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};
	
	buffer.ChangeData(&toGPU);
	buffer.BindPipeline();
}

void Unlit_Material::BindUnlit_T(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindUnlit(subMesh, renderer);
	
	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	
	texture->BindPipeline(Texture::Type::DIFFUSE);
}
