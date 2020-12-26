#pragma once
#include "Material.h"

class Phong_Material : public Material
{
#ifdef EDITOR
	friend class EditorMaterialEditWindow;
#endif

public:
	Phong_Material(std::string name);

	void Bind(const Mesh::SubMesh* subMesh, const Renderer* renderer) const override;

	//Diffuse
	void SetDiffuseColor(const DirectX::XMFLOAT4& color);
	void SetDiffuseMap(const Texture* texture);
	bool HasDiffuseMap() const;
	const Texture* GetDiffuseMap() const;


	//Specular
	//If material has specular map specular color will have no effect.
	void SetSpecularColor(const DirectX::XMFLOAT4& color);
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

	//Diffuse color = 0.8,0.8,0.8,1.0
	//Diffuse map = null
	//Specular color = 1.0,1.0,1.0,1.0
	//Specular map = null
	//Shininess = 0.5
	//Shininess Strength = 0.5
	static const Phong_Material* GetDefaultMaterial();

private:
	//Binds default phong material, only diffuse and specular color, no texture mapping.
	void BindPhong(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with diffuse map.
	void BindPhong_D(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with specular map.
	void BindPhong_S(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with diffuse and specular map.
	void BindPhong_D_S(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with normal map.
	void BindPhong_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with diffuse and normal map.
	void BindPhong_D_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	//Phong with diffuse,specular and normal map.
	void BindPhong_D_S_N(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	void DecideBind();

private:
	void (Phong_Material::* Bind_)(const Mesh::SubMesh*,const Renderer*) const = &Phong_Material::BindPhong;

	DirectX::XMFLOAT4 diffuseColor = { 0.8f, 0.8f, 0.8f, 1.0f };
	const Texture* diffuseMap = nullptr;

	DirectX::XMFLOAT4 specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	const Texture* specularMap = nullptr;
	float Shininess = 0.5f;
	float ShininessStrength = 0.5f;

	const Texture* normalMap = nullptr;

private:
	enum VariantFlag
	{
		None =  0,
		D    =  1, //Diffuse
		S    =  2, //Specular
		DS   =  3, //Diffuse and specular
		N    =  4, //Normal
		DN   =  5, //Diffuse and normal
		DSN  =  7  //Diffuse, specular and normal
	}vFlag = VariantFlag::None;
};
