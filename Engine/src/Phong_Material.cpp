#include "Phong_Material.h"
#include "Resources.h"
#include "Renderer.h"
#include "Graphics.h"

Phong_Material::Phong_Material(std::string name)
	:Material(name, Resources::FindShader("Phong"))
{
}

void Phong_Material::Bind(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	(this->*(this->Bind_))(subMesh, renderer);
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
	{
		shaderView.ActivateMacro_PS("DIFFUSETEXTURE");
		shaderView.ActivateMacro_VS("DIFFUSETEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::D);
	}
	else
	{
		shaderView.DeactivateMacro_PS("DIFFUSETEXTURE");
		shaderView.DeactivateMacro_VS("DIFFUSETEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::D);
	}

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
	{
		shaderView.ActivateMacro_PS("SPECULARTEXTURE");
		shaderView.ActivateMacro_VS("SPECULARTEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::S);
	}
	else
	{
		shaderView.DeactivateMacro_PS("SPECULARTEXTURE");
		shaderView.DeactivateMacro_VS("SPECULARTEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::S);
	}

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
	{
		shaderView.ActivateMacro_PS("NORMALTEXTURE");
		shaderView.ActivateMacro_VS("NORMALTEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag | VariantFlag::N);
	}
	else
	{
		shaderView.DeactivateMacro_PS("NORMALTEXTURE");
		shaderView.DeactivateMacro_VS("NORMALTEXTURE");
		vFlag = static_cast<VariantFlag>(vFlag & ~VariantFlag::N);
	}

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

void Phong_Material::BindPhong(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	//Binding shaders
	shaderView.Bind();

	//Lights
	renderer->UpdateDirectionalLightBuffer();
	renderer->UpdatePointLightBuffer();
	
	//Binding mesh
	subMesh->GetVertexElement(VertexBuffer::ElementType::Position3D)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Normal)->BindPipeline();
	subMesh->GetIndexBuffer()->BindPipeline();

	//Vertex Shader
	static VS_ConstantBuffer<VS_CB_Slot0> VS_CB = {
		&VS_CB_Slot0_,
		1u,
		0u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};
	
	VS_CB_Slot0_.modelView = renderer->GetWorldViewMatrix();
	VS_CB_Slot0_.normalMatrix = renderer->GetNormalMatrix();
	VS_CB_Slot0_.MVP = renderer->GetWorldViewProjectionMatrix();

	VS_CB.ChangeData(&VS_CB_Slot0_);
	VS_CB.BindPipeline();

	//Pixel Shader	
	static PS_ConstantBuffer<PS_CB_Slot3> PS_CB = {
		&PS_CB_Slot3_,
		1u,
		PS_MaterialSlot,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true
	};

	PS_CB_Slot3_.diffuseColor = diffuseColor;
	PS_CB_Slot3_.specularColor = specularColor;
	PS_CB_Slot3_.shininess = Shininess;
	PS_CB_Slot3_.shininessStrength = ShininessStrength;

	PS_CB.ChangeData(&PS_CB_Slot3_);
	PS_CB.BindPipeline();
}

void Phong_Material::BindPhong_D(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	BindPhong(subMesh, renderer);

	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_S(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	BindPhong(subMesh, renderer);

	specularMap->BindPipeline(Texture::Type::SPECULAR);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_D_S(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	BindPhong(subMesh, renderer);

	specularMap->BindPipeline(Texture::Type::SPECULAR);
	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
}

void Phong_Material::BindPhong_N(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	BindPhong(subMesh, renderer);

	normalMap->BindPipeline(Texture::Type::NORMAL);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Phong_Material::BindPhong_D_N(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
{
	BindPhong(subMesh, renderer);

	diffuseMap->BindPipeline(Texture::Type::DIFFUSE);
	normalMap->BindPipeline(Texture::Type::NORMAL);

	subMesh->GetVertexElement(VertexBuffer::ElementType::TexCoord)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Tangent)->BindPipeline();
	subMesh->GetVertexElement(VertexBuffer::ElementType::Bitangent)->BindPipeline();
}

void Phong_Material::BindPhong_D_S_N(const Mesh::SubMesh* subMesh, Renderer*  renderer) const
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
		break;
	case VariantFlag::D:
		Bind_ = &Phong_Material::BindPhong_D;
		break;
	case VariantFlag::S:
		Bind_ = &Phong_Material::BindPhong_S;
		break;
	case VariantFlag::DS:
		Bind_ = &Phong_Material::BindPhong_D_S;
		break;
	case VariantFlag::N:
		Bind_ = &Phong_Material::BindPhong_N;
		break;
	case VariantFlag::DN:
		Bind_ = &Phong_Material::BindPhong_D_N;
		break;
	case VariantFlag::DSN:
		Bind_ = &Phong_Material::BindPhong_D_S_N;
		break;
	default:
		break;
	}
}