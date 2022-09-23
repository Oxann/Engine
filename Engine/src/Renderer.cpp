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

void Renderer::Start()
{
	Scene::GetActiveScene()->rendererManager.renderers.push_back(this);
	rendererManager = &Scene::GetActiveScene()->rendererManager;
}

void Renderer::Render(unsigned int subMeshIndex)
{
	Render(subMeshIndex, materials[subMeshIndex]);
}

void Renderer::Render(unsigned int subMeshIndex, const Material* material)
{
	if (subMeshIndex < mesh->GetSubMeshCount())
	{
		const Mesh::SubMesh* subMesh = &GetMesh()->GetSubMeshes()[subMeshIndex];
		Material* material = materials[subMeshIndex];
		
		subMesh->GetIndexBuffer()->BindPipeline();

		//Updating vertex shader per object buffer for this renderer.
		Shader::VertexShaderPerObjectBuffer::buffer.model = GetWorldMatrix();
		Shader::VertexShaderPerObjectBuffer::buffer.modelView = GetWorldViewMatrix();
		Shader::VertexShaderPerObjectBuffer::buffer.modelViewProjection = GetWorldViewProjectionMatrix();
		Shader::VertexShaderPerObjectBuffer::buffer.normal = GetNormalMatrix();
		Shader::GetVertexShaderPerObjectBuffer()->ChangeData(&Shader::VertexShaderPerObjectBuffer::buffer);

		if (material->HasShadows())
		{
			Shader::VertexShaderShadowBuffer::buffer.lightSpaceMatrix = lightSpaceMatrix;
			Shader::GetVertexShaderShadowBuffer()->ChangeData(&Shader::VertexShaderShadowBuffer::buffer);
		}

		materials[subMeshIndex]->Bind(subMesh);
		Graphics::pDeviceContext->DrawIndexed(subMesh->GetIndexCount(), 0u, 0u);
	}
}

Renderer* Renderer::Clone()
{
	Renderer* newRenderer = new Renderer;
	newRenderer->materials = this->materials;
	newRenderer->mesh = this->mesh;
	return newRenderer;
}

void Renderer::PrepareForRendering()
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

void Renderer::UpdateMatrices()
{
	worldMatrix = GetTransform()->GetLocalMatrix();

	for (const Entity* parent = GetEntity()->GetParent(); parent != nullptr;)
	{
		//If the parent entity has a renderer we can use its world matrix.
		if (Renderer* parentRenderer = parent->GetComponent<Renderer>())
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
		normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, GetWorldMatrix()));
		isNormalMatrixUpdated = true;
	}

	return normalMatrix;
}

void Renderer::SetMesh(Mesh* mesh)
{
	static Material* defaultLitMaterial = Resources::FindMaterial("$Default\\LitPBR");

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
