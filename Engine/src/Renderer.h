
#pragma once
#include "Mesh.h"
#include "Transform.h"
#include <DirectXMath.h>
#include "Component.h"
#include "Shader.h"
#include "Texture.h"
#include "Resources.h"
#include "Material.h"

class Renderer final: public Component
{
#ifdef EDITOR
	friend class EditorEntityWindow;
#endif

public:
	void Start() override;

	void Update() override;

	Renderer* Clone() override;

	//If renderer already has a mesh this function also resets all materials to default.
	void SetMesh(const Mesh* mesh);

	const Mesh* GetMesh() const;

	//materialIndex = index of the sub mesh, first index is 0.
	void SetMaterial(const Material* material, unsigned int materialIndex);
	
	const std::vector<const Material*>& GetMaterials() const;
private:
	void UpdateDirectionalLightBuffer();
	void UpdatePointLightBuffer();
	void UpdateTransformBuffer();

public:
	D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
private:
	const Mesh* mesh = nullptr;
	std::vector<const Material*> materials;
	Transform* transform;
};