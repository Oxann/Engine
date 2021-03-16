#include "Unlit_Material.h"
#include "Resources.h"
#include "Renderer.h"
#include "Graphics.h"

Unlit_Material::Unlit_Material(std::string name)
	:Material(name)
{
	vertexShader = Resources::FindVertexShader("Unlit.cso");
	pixelShader = Resources::FindPixelShader("Unlit.cso");
}

void Unlit_Material::Bind(const Mesh::SubMesh* subMesh, Renderer* renderer) const
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

void Unlit_Material::BindUnlit(const Mesh::SubMesh* subMesh, Renderer* renderer) const
{
	//Binding shaders
	vertexShader->BindPipeline();
	pixelShader->BindPipeline();

	//Binding mesh
	subMesh->GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	subMesh->GetIndexBuffer()->BindPipeline();

	//Vertex Shader
	static VS_ConstantBuffer<DirectX::XMMATRIX> VS_CB = {
		&renderer->GetWorldViewProjectionMatrix(),
		1u,
		0u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};

	VS_CB.ChangeData(&renderer->GetWorldViewProjectionMatrix());
	VS_CB.BindPipeline();

	//Pixel shader
	static PS_ConstantBuffer<PS_CB_Slot3> PS_CB = {
		&PS_CB_Slot3_,
		1u,
		PS_MaterialSlot,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};
	
	PS_CB_Slot3_.color = color;

	PS_CB.ChangeData(&PS_CB_Slot3_);
	PS_CB.BindPipeline();
}

void Unlit_Material::BindUnlit_T(const Mesh::SubMesh* subMesh, Renderer* renderer) const
{
	BindUnlit(subMesh, renderer);
	
	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	
	texture->BindPipeline(Texture::Type::DIFFUSE);
}
