#pragma once
#include "Material.h"

#include <DirectXMath.h>



class Unlit_Material : public Material
{
#ifdef EDITOR
	friend class EditorMaterialEditWindow;
#endif

public:
	Unlit_Material(std::string name);

	void Bind(const Mesh::SubMesh* subMesh, const Renderer* renderer) const override;

	void SetColor(const DirectX::XMFLOAT4& color);

	void SetTexture(const Texture* texture);

	const Texture* GetTexture() const;

	bool HasTexture() const;

	//Color = {0.8, 0.8, 0.8, 1.0}
	//Texture = null
	static const Material* GetDefaultMaterial();

private:
	void BindUnlit(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;

	void BindUnlit_T(const Mesh::SubMesh* subMesh, const Renderer* renderer) const;
private:
	void (Unlit_Material::* Bind_)(const Mesh::SubMesh*, const Renderer*) const = &Unlit_Material::BindUnlit;

	DirectX::XMFLOAT4 color = {0.8f,0.8f,0.8f,1.0f};
	const Texture* texture = nullptr;
};