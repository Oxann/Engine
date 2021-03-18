
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
	friend class RendererManager;
	friend class Phong_Material;
	friend class Unlit_Material;
	friend class Engine;
	friend class Graphics;
	friend class RenderQueueOutline;
#ifdef EDITOR
	friend class EditorEntityWindow;
	friend class Editor;
#endif

public:
	Renderer* Clone() override;

	//If renderer already has a mesh this function also resets all materials to default.
	void SetMesh(const Mesh* mesh);

	const Mesh* GetMesh() const;

	//materialIndex = index of the sub mesh, first index is 0.
	void SetMaterial(const Material* material, unsigned int materialIndex);
	
	const std::vector<const Material*>& GetMaterials() const;

	//This functions are used by Render Queues. DON'T CALL FROM ELSEWEHERE.
	//Binded material will be used.
	void Render(unsigned int subMeshIndex);

	//Custom material needed.
	void Render(unsigned int subMeshIndex, const Material* material);

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldViewMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldViewProjectionMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetNormalMatrix();
private:
	void Start();
	void Update() override;
	void UpdateDirectionalLightBuffer() const;
	void UpdatePointLightBuffer() const;
public:
	D3D_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
private:
	const Mesh* mesh = nullptr;
	std::vector<const Material*> materials;

	//All matrices are column order.
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX worldViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX worldViewProjectionMatrix = DirectX::XMMatrixIdentity();

	bool isWorldMatrixUpdated = false;
	bool isWorldViewMatrixUpdated = false;
	bool isWorldViewProjectionMatrixUpdated = false;
	bool isNormalMatrixUpdated = false;
};