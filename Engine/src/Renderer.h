
#pragma once
#include "Mesh.h"
#include <DirectXMath.h>
#include "Shader.h"
#include "Texture.h"
#include "Resources.h"
#include "Material.h"
#include "RendererBase.h"


class Renderer final: public RendererBase
{
	friend Material;
	friend RendererManager;

#ifdef EDITOR
	friend class EditorEntityWindow;
	friend class Editor;
#endif

public:
	void Start() override;

	//This functions are used by Render Queues. DON'T CALL FROM ELSEWEHERE.
	//Binded material will be used.
	virtual void Render(unsigned int subMeshIndex) override;

	//Custom material needed.
	virtual void Render(unsigned int subMeshIndex, const Material* material) override;

	//If renderer already has a mesh this function also resets all materials to default.
	void SetMesh(Mesh* mesh);

	const Mesh* GetMesh() const;



	Renderer* Clone() override;

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldViewMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetWorldViewProjectionMatrix();

	//COLUMN ORDER!!!
	const DirectX::XMMATRIX& GetNormalMatrix();
private:
	void PrepareForRendering() override;
	void UpdateMatrices();
	void TransposeMatrices();
	void SetLightSpaceMatrix(const DirectX::XMMATRIX lightSpaceMatrix);
private:
	Mesh* mesh = nullptr;

	//All matrices are column order after renderer updated.
	DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX worldViewMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX normalMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX worldViewProjectionMatrix = DirectX::XMMatrixIdentity();
	DirectX::XMMATRIX lightSpaceMatrix = DirectX::XMMatrixIdentity();

	bool isWorldMatrixUpdated = false;
	bool isWorldViewMatrixUpdated = false;
	bool isWorldViewProjectionMatrixUpdated = false;
	bool isNormalMatrixUpdated = false;
};