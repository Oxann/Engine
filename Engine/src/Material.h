#pragma once
#include <DirectXMath.h>
#include "Texture.h"
#include "ConstantBuffer.h"
#include "Shader.h"
#include "Mesh.h"


class Material : public ResourceBase
{
	friend class Renderer;

#ifdef EDITOR
	friend class EditorMaterialEditWindow;
#endif

public:
	Material(std::string name);

	//Diffuse
	void SetDiffuseColor(DirectX::XMFLOAT4 color);
	void SetDiffuseMap(const Texture* texture);
	bool HasDiffuseMap() const;
	const Texture* GetDiffuseMap() const;


	//Specular
	//If material has specular map specular color will have no effect.
	void SetSpecularColor(DirectX::XMFLOAT4 color);
	void SetSpecularMap(const Texture* texture);
	bool HasSpecularMap() const;
	const Texture* GetSpecularMap() const;

	//Normal map
	void SetNormalMap(const Texture* texture);
	const Texture* GetNormalMap() const;
	bool HasNormalMap() const;

	//[0,1]
	void SetShininess(float shininess);
	//[0,1]
	float GetShininess() const;

	void SetShininessStrength(float sstrength);
	float GetShininessStrength();

	//Diffuse color = 1.0,1.0,1.0,1.0
	//Diffuse map = null
	//Specular color = 1.0,1.0,1.0,1.0
	//Specular map = null
	//Shininess = 0.5
	static const Material* GetDefaultMaterial();

private:
	//Binds default phong material, only diffuse and specular color, no texture mapping.
	void BindPhong(const Mesh::SubMesh* mesh) const;

	//Phong with diffuse map.
	void BindPhong_D(const Mesh::SubMesh* mesh) const;

	//Phong with specular map.
	void BindPhong_S(const Mesh::SubMesh* mesh) const;

	//Phong with diffuse and specular map.
	void BindPhong_D_S(const Mesh::SubMesh* mesh) const;

	//Phong with diffuse,specular and normal map.
	void BindPhong_D_S_N(const Mesh::SubMesh* mesh) const;

	void DecideBind();
private:

	void (Material::* Bind)(const Mesh::SubMesh*) const = &Material::BindPhong;

	DirectX::XMFLOAT4 diffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	const Texture* diffuseMap = nullptr;

	DirectX::XMFLOAT4 specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Texture* specularMap = nullptr;
	float Shininess = 0.5f;
	float ShininessStrength = 0.5f;

	const Texture* normalMap = nullptr;

	//Shaders
	VertexShader* vertexShader;
	PixelShader* pixelShader;
	static constexpr unsigned int PS_MaterialSlot = 3u;

private:
	enum MappingFlag
	{
		None = 0,
		D = 1, //Diffuse
		S = 2, //Specular
		DS = 3, //Diffuse and specular
		N = 4,
		DSN = 7
	}mFlag = MappingFlag::None;
};