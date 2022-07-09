#include "Renderer.h"
#include "Entity.h"
#include "Graphics.h"
#include "EngineException.h"
#include "Light.h"
#include "EngineAssert.h"
#include "Transform.h"
#include "Scene.h"
#include "Editor.h"
#include "Resources.h"

Renderer* Renderer::Clone()
{
	Renderer* newRenderer = new Renderer;
	newRenderer->topology = this->topology;
	newRenderer->materials = this->materials;
	newRenderer->mesh = this->mesh;
	return newRenderer;
}

void Renderer::SetMesh(Mesh* mesh)
{
	static Material* defaultLitMaterial = Resources::FindMaterial("$Default\\Lit");

	this->mesh = mesh;
	materials.resize(0u);

	if (mesh != nullptr)
	{
		for (int i = 0; i < mesh->GetSubMeshCount(); i++)
			materials.push_back(defaultLitMaterial);
	}
}

const Mesh* Renderer::GetMesh() const
{
	return mesh;
}

void Renderer::SetMaterial(Material* material, unsigned int materialIndex)
{
	if (materialIndex >= materials.size())
	{
		std::stringstream ss;
		ss << "Entity name: " << GetEntity()->name << std::endl
			<< "Component: Renderer" << std::endl
			<< "Material index is not correct, it must be greater than zero and less than submeshcount - 1";
		THROW_ENGINE_EXCEPTION(ss.str(), true);
	}
	else
		materials[materialIndex] = material;
}

const std::vector<Material*>& Renderer::GetMaterials() const
{
	return materials;
}

void Renderer::Start()
{
	Scene::GetActiveScene()->rendererManager.renderers.push_back(this);
	rendererManager = &Scene::GetActiveScene()->rendererManager;
}

void Renderer::Update()
{
	UpdateMatrices();

	//Frustum culling on merged AABB.
	if (mesh->GetSubMeshCount() > 1)
	{
		DirectX::BoundingBox viewSpaceAABB;
		mesh->AABB.Transform(viewSpaceAABB, worldViewMatrix);
		
		//If merged AABB is not inside the frustum we can return here.
		if (Scene::GetActiveScene()->rendererManager.frustum.Contains(viewSpaceAABB) == DirectX::ContainmentType::DISJOINT)
		{
			TransposeMatrices();
			return;
		}
	}

	//Passing to render queues
	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
	{
		//Frustum culling on submesh.
		DirectX::BoundingBox viewSpaceAABB;
		mesh->GetSubMeshes()[i].AABB.Transform(viewSpaceAABB, worldViewMatrix);

		if (Scene::GetActiveScene()->rendererManager.frustum.Contains(viewSpaceAABB) != DirectX::ContainmentType::DISJOINT)
		{
#ifdef EDITOR
			if (Editor::isWireframeEnabled)
			{
				Scene::GetActiveScene()->rendererManager.renderQueueWireframe.Add(this, i);
				continue;
			}
#endif

			if (materials[i]->mode == Material::Mode::Opaque)
			{
				Scene::GetActiveScene()->rendererManager.renderQueueOpaque.Add(this, i);
			}
			else
			{
				Scene::GetActiveScene()->rendererManager.renderQueueTransparent.Add(this, i);
			}
		}
	}

	//Matrices need to be converted to column order before passing them to shaders.
	TransposeMatrices();
}

void Renderer::Render(unsigned int subMeshIndex)
{
	if (subMeshIndex < mesh->GetSubMeshCount())
	{
		materials[subMeshIndex]->Bind(&mesh->GetSubMeshes()[subMeshIndex], this);
		Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[subMeshIndex].GetIndexCount(), 0u, 0u);
	}
}

void Renderer::Render(unsigned int subMeshIndex, const Material* material)
{
	if (subMeshIndex < mesh->GetSubMeshCount())
	{
		material->Bind(&mesh->GetSubMeshes()[subMeshIndex], this);
		Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[subMeshIndex].GetIndexCount(), 0u, 0u);
	}
}

void Renderer::UpdateMatrices()
{
	worldMatrix = GetTransform()->GetLocalMatrix();

	for (const Entity* parent = GetEntity()->GetParent(); parent != nullptr;)
	{
		//If the parent entity has a renderer we can use its world matrix.
		if (Renderer* parentRenderer = parent->GetRenderer())
		{
			worldMatrix = worldMatrix * parentRenderer->worldMatrix;
			break;
		}
		else
		{
			worldMatrix = worldMatrix * parent->GetTransform()->GetLocalMatrix();
			parent = parent->GetParent();
		}
	}

	worldViewMatrix = worldMatrix * Graphics::viewMatrix;
	worldViewProjectionMatrix = worldViewMatrix * Graphics::projectionMatrix;

	isNormalMatrixUpdated = false;
}

void Renderer::TransposeMatrices()
{
	worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	worldViewMatrix = DirectX::XMMatrixTranspose(worldViewMatrix);
	worldViewProjectionMatrix = DirectX::XMMatrixTranspose(worldViewProjectionMatrix);
}

void Renderer::SetLightSpaceMatrix(const DirectX::XMMATRIX lightSpaceMatrix)
{
	this->lightSpaceMatrix = lightSpaceMatrix;
}

const DirectX::XMMATRIX& Renderer::GetWorldMatrix()
{
	return worldMatrix;
}

const DirectX::XMMATRIX& Renderer::GetWorldViewMatrix()
{
	return worldViewMatrix;
}

const DirectX::XMMATRIX& Renderer::GetWorldViewProjectionMatrix()
{
	return worldViewProjectionMatrix;;
}

const DirectX::XMMATRIX& Renderer::GetNormalMatrix()
{
	if (!isNormalMatrixUpdated)
	{
		//Matrices need to be converted to column order before passing them to shaders.
		normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GetWorldViewMatrix()));
		isNormalMatrixUpdated = true;
	}

	return normalMatrix;
}