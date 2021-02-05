
#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include "Shader.h"
#include "Texture.h"
#include "Resources.h"
#include "Material.h"
#include "Component.h"

class Renderer final: public Component
{
	friend class Phong_Material;
	friend class Unlit_Material;
	friend class Engine;

#ifdef EDITOR
	friend class EditorEntityWindow;
#endif

public:
	Renderer* Clone() override;

	//If renderer already has a mesh this function also resets all materials to default.
	void SetMesh(const Mesh* mesh);

	const Mesh* GetMesh() const;

	//materialIndex = index of the sub mesh, first index is 0.
	void SetMaterial(const Material* material, unsigned int materialIndex);
	
	const std::vector<const Material*>& GetMaterials() const;
private:
	void UpdateDirectionalLightBuffer() const;
	void UpdatePointLightBuffer() const;
public:
	D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
private:
	const Mesh* mesh = nullptr;
	std::vector<const Material*> materials;

	//All matrices are column order.
	DirectX::XMMATRIX MV_Matrix;
	DirectX::XMMATRIX normalMatrix;
	DirectX::XMMATRIX MVP_Matrix;
};