#include "Phong_Material.h"
#include "Resources.h"



Phong_Material::Phong_Material(std::string name)
	:Material(name)
{
	vertexShader = Resources::FindVertexShader("Phong.cso");
	pixelShader = Resources::FindPixelShader("Phong.cso");
}

void Phong_Material::Bind(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	(this->*(this->Bind_))(subMesh,renderer);
}

void Phong_Material::SetDiffuseColor(const DirectX::XMFLOAT4& color)
{
	diffuseColor = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f),
		std::clamp(color.w, 0.0f, 1.0f)
	};
}

void Phong_Material::SetDiffuseMap(const Texture* texture)
{
	diffuseMap = texture;

	if (diffuseMap)
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::D);
	else
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::D);

	DecideBind();
}

bool Phong_Material::HasDiffuseMap() const
{
	return diffuseMap;
}

const Texture* Phong_Material::GetDiffuseMap() const
{
	return diffuseMap;
}

void Phong_Material::SetSpecularColor(const DirectX::XMFLOAT4& color)
{
	specularColor = {
		std::clamp(color.x, 0.0f, 1.0f),
		std::clamp(color.y, 0.0f, 1.0f),
		std::clamp(color.z, 0.0f, 1.0f),
		std::clamp(color.w, 0.0f, 1.0f)
	};
}

void Phong_Material::SetSpecularMap(const Texture* texture)
{
	specularMap = texture;

	if (specularMap)
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::S);
	else
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::S);

	DecideBind();
}

bool Phong_Material::HasSpecularMap() const
{
	return specularMap;
}

const Texture* Phong_Material::GetSpecularMap() const
{
	return specularMap;
}

void Phong_Material::SetNormalMap(const Texture* texture)
{
	normalMap = texture;

	if (normalMap)
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::N);
	else
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::N);

	DecideBind();
}

const Texture* Phong_Material::GetNormalMap() const
{
	return normalMap;
}

bool Phong_Material::HasNormalMap() const
{
	return normalMap;
}

void Phong_Material::SetShininess(float shininess)
{
	Shininess = std::clamp(shininess, 0.0f, 1.0f);
}

float Phong_Material::GetShininess() const
{
	return Shininess;
}

void Phong_Material::SetShininessStrength(float sstrength)
{
	ShininessStrength = sstrength;
}

float Phong_Material::GetShininessStrength()
{
	return ShininessStrength;
}

const Phong_Material* Phong_Material::GetDefaultMaterial()
{
	static Phong_Material defaultMat("PhongDefaultMat");
	return &defaultMat;
}

void Phong_Material::BindPhong(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	//Binding shaders
	vertexShader->BindPipeline();
	pixelShader->BindPipeline();
	
	//Binding lights
	renderer->UpdateDirectionalLightBuffer();
	renderer->UpdatePointLightBuffer();

	//Binding transform
	renderer->UpdateTransformBuffer();

	//Binding mesh
	subMesh->GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Normal)->BindPipeline();
	subMesh->GetIndexBuffer()->BindPipeline();

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
	buffer.BindPipeline();
}

void Phong_Material::BindPhong_D(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh,renderer);

	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_S(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh, renderer);

	specularMap->BindPipeline(Texture::Type::SPECULAR);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_D_S(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh, renderer);

	specularMap->BindPipeline(Texture::Type::SPECULAR);
	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh, renderer);

	normalMap->BindPipeline(Texture::Type::NORMAL);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Phong_Material::BindPhong_D_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh, renderer);

	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);
	normalMap->BindPipeline(Texture::Type::NORMAL);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Phong_Material::BindPhong_D_S_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const
{
	BindPhong(subMesh, renderer);

	specularMap->BindPipeline(Texture::Type::SPECULAR);
	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);
	normalMap->BindPipeline(Texture::Type::NORMAL);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Phong_Material::DecideBind()
{
	switch (vFlag)
	{
	case VariantFlag::None:
		Bind_ = &Phong_Material::BindPhong;
		vertexShader = Resources::FindVertexShader("Phong.cso");
		pixelShader = Resources::FindPixelShader("Phong.cso");
		break;
	case VariantFlag::D:
		Bind_ = &Phong_Material::BindPhong_D;
		vertexShader = Resources::FindVertexShader("Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("Phong_D.cso");
		break;
	case VariantFlag::S:
		Bind_ = &Phong_Material::BindPhong_S;
		vertexShader = Resources::FindVertexShader("Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("PS_Phong_S.cso");
		break;
	case VariantFlag::DS:
		Bind_ = &Phong_Material::BindPhong_D_S;
		vertexShader = Resources::FindVertexShader("Phong_D_S.cso");
		pixelShader = Resources::FindPixelShader("Phong_D_S.cso");
		break;
	case VariantFlag::N:
		Bind_ = &Phong_Material::BindPhong_N;
		vertexShader = Resources::FindVertexShader("Phong_D_S_N.cso");
		pixelShader = Resources::FindPixelShader("Phong_N.cso");
		break;
	case VariantFlag::DN:
		Bind_ = &Phong_Material::BindPhong_D_N;
		vertexShader = Resources::FindVertexShader("Phong_D_S_N.cso");
		pixelShader = Resources::FindPixelShader("Phong_D_N.cso");
		break;
	case VariantFlag::DSN:
		Bind_ = &Phong_Material::BindPhong_D_S_N;
		vertexShader = Resources::FindVertexShader("Phong_D_S_N.cso");
		pixelShader = Resources::FindPixelShader("Phong_D_S_N.cso");
		break;
	default:
		break;
	}
}