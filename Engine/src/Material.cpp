#include "Material.h"
#include "Input.h"
#include "Resources.h"



Material::Material(std::string name)
	:ResourceBase(name)
{
	vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong.cso");
	pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong.cso");
}

void Material::SetDiffuseColor(DirectX::XMFLOAT4 color)
{
	diffuseColor = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f),
		std::clamp(color.w, 0.0f, 1.0f)
	};
}

void Material::SetDiffuseMap(const Texture* texture)
{
	diffuseMap = texture;
	
	if (diffuseMap)
		mFlag = static_cast<MappingFlag>(mFlag | MappingFlag::D);
	else
		mFlag = static_cast<MappingFlag>(mFlag & ~MappingFlag::D);

	DecideBind();
}

bool Material::HasDiffuseMap() const
{
	return diffuseMap;
}

const Texture* Material::GetDiffuseMap() const
{
	return diffuseMap;
}

void Material::SetSpecularColor(DirectX::XMFLOAT4 color)
{
	specularColor = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f),
		std::clamp(color.w, 0.0f, 1.0f)
	};
}

void Material::SetSpecularMap(const Texture* texture)
{
	specularMap = texture;
	
	if (specularMap)
		mFlag = static_cast<MappingFlag>(mFlag | MappingFlag::S);
	else
		mFlag = static_cast<MappingFlag>(mFlag & ~MappingFlag::S);

	DecideBind();
}

bool Material::HasSpecularMap() const
{
	return specularMap;
}

const Texture* Material::GetSpecularMap() const
{
	return specularMap;
}

void Material::SetNormalMap(const Texture* texture)
{
	normalMap = texture;

	if (normalMap)
		mFlag = static_cast<MappingFlag>(mFlag | MappingFlag::N);
	else
		mFlag = static_cast<MappingFlag>(mFlag & ~MappingFlag::N);

	DecideBind();
}

const Texture* Material::GetNormalMap() const
{
	return normalMap;
}

bool Material::HasNormalMap() const
{
	return normalMap;
}

void Material::SetShininess(float shininess)
{
	Shininess = std::clamp(shininess, 0.0f, 1.0f);
}

float Material::GetShininess() const
{
	return Shininess;
}

void Material::SetShininessStrength(float sstrength)
{
	ShininessStrength = sstrength;
}

float Material::GetShininessStrength()
{
	return ShininessStrength;
}

const Material* Material::GetDefaultMaterial()
{
	static Material defaultMat("defaultMat");
	return &defaultMat;
}

void Material::BindPhong(const Mesh::SubMesh* mesh) const
{
	//Binding shaders
	vertexShader->BindPipeline();
	pixelShader->BindPipeline();

	//Binding mesh 
	mesh->GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	mesh->GetVertexElement(VertexBuffer::ElementType::Normal)->BindPipeline();
	mesh->GetIndexBuffer()->BindPipeline();
	
	//Binding material properties
	struct Material_TO_GPU
	{
		alignas(16u)DirectX::XMFLOAT4 diffuseColor;
		alignas(16u)DirectX::XMFLOAT4 specularColor;
		float shininess;
		float shininessStrength;
	};

	static Material_TO_GPU toGPU = {};
	toGPU.diffuseColor = diffuseColor;
	toGPU.specularColor = specularColor;
	toGPU.shininess = Shininess;
	toGPU.shininessStrength = ShininessStrength;


	static PS_ConstantBuffer<Material_TO_GPU> buffer = {
	&toGPU,
	1u,
	PS_MaterialSlot,
	D3D11_USAGE::D3D11_USAGE_DYNAMIC,
	D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
	true
	};
	buffer.ChangeData(&toGPU);
}

void Material::BindPhong_D(const Mesh::SubMesh* mesh) const
{
	BindPhong(mesh);

	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	mesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Material::BindPhong_S(const Mesh::SubMesh* mesh) const
{
	BindPhong(mesh);

	specularMap->BindPipeline(Texture::Type::SPECULAR);

	mesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Material::BindPhong_D_S(const Mesh::SubMesh* mesh) const
{
	BindPhong(mesh);

	specularMap->BindPipeline(Texture::Type::SPECULAR);
	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	mesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Material::BindPhong_D_S_N(const Mesh::SubMesh* mesh) const
{
	BindPhong(mesh);

	specularMap->BindPipeline(Texture::Type::SPECULAR);
	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);
	normalMap->BindPipeline(Texture::Type::NORMAL);

	mesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	mesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	mesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Material::DecideBind()
{
	switch (mFlag)
	{
	case MappingFlag::None:
		Bind = &Material::BindPhong;
		vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong.cso");
		pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong.cso");
		break;
	case MappingFlag::D:
		Bind = &Material::BindPhong_D;
		vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong_D.cso");
		break;
	case MappingFlag::S:
		Bind = &Material::BindPhong_S;
		vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong_S.cso");
		break;
	case MappingFlag::DS:
		Bind = &Material::BindPhong_D_S;
		vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong_D_S.cso");
		break;
	case MappingFlag::DSN:
		Bind = &Material::BindPhong_D_S_N;
		vertexShader = Resources::FindVertexShader("Shaders\\Debug\\VS_Phong_D_S_N.cso");
		pixelShader = Resources::FindPixelShader("Shaders\\Debug\\PS_Phong_D_S_N.cso");
		break;
	default:
		break;
	}
}