#include "Renderer.h"
#include "Entity.h"
#include "Graphics.h"
#include "EngineException.h"
#include "Light.h"
#include "EngineAssert.h"
#include "Phong_Material.h"
#include "Transform.h"
#include "Scene.h"
#include "Editor.h"

Renderer* Renderer::Clone()
{
	Renderer* newRenderer = new Renderer;
	newRenderer->topology = this->topology;
	newRenderer->materials = this->materials;
	newRenderer->mesh = this->mesh;
	return newRenderer;
}

void Renderer::SetMesh(const Mesh* mesh)
{
	this->mesh = mesh;
	materials.resize(0u);

	if (mesh != nullptr)
	{
		for (int i = 0; i < mesh->GetSubMeshCount(); i++)
			materials.push_back(Phong_Material::GetDefaultMaterial());
	}
}

const Mesh* Renderer::GetMesh() const
{
	return mesh;
}

void Renderer::SetMaterial(const Material* material, unsigned int materialIndex)
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

const std::vector<const Material*>& Renderer::GetMaterials() const
{
	return materials;
}

void Renderer::Start()
{
	Scene::GetActiveScene()->rendererManager.renderers.push_back(this);
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
			return;
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
	materials[subMeshIndex]->Bind(&mesh->GetSubMeshes()[subMeshIndex], this);
	Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[subMeshIndex].GetIndexCount(), 0u, 0u);
}

void Renderer::Render(unsigned int subMeshIndex, const Material* material)
{
	material->Bind(&mesh->GetSubMeshes()[subMeshIndex], this);
	Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[subMeshIndex].GetIndexCount(), 0u, 0u);
}

void Renderer::UpdateDirectionalLightBuffer() const
{
	struct DirectionalLights_TO_GPU
	{
		alignas(16) unsigned int Count;
		struct PerLightInfo
		{
			alignas(16) DirectX::XMFLOAT3 ligth; //color * intensity
			alignas(16) DirectX::XMFLOAT3 direction;
		};
		PerLightInfo lights[4];
	};

	static DirectionalLights_TO_GPU toGPU;	
	toGPU.Count = DirectionalLight::lights.size();

	for (unsigned int i = 0; i < DirectionalLight::MaxCount && i < toGPU.Count; i++)
	{
		const DirectionalLight& currentLight = *DirectionalLight::lights[i];

		DirectX::XMMATRIX lightModelView = DirectX::XMMatrixMultiply(
			DirectX::XMMatrixRotationQuaternion(currentLight.GetEntity()->GetTransform()->GetWorldQuaternion()), Graphics::viewMatrix);
		
		DirectX::XMStoreFloat3(&toGPU.lights[i].direction,DirectX::XMVector3Normalize(lightModelView.r[2]));
		
		toGPU.lights[i].ligth = { currentLight.color.x * currentLight.intensity,
									currentLight.color.y * currentLight.intensity,
									currentLight.color.z * currentLight.intensity };
	}

	//First time initialization
	static PS_ConstantBuffer<DirectionalLights_TO_GPU> directionalLightBuffer = {
		&toGPU,
		1u,
		1u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true
	};
		
	directionalLightBuffer.ChangeData(&toGPU);
}

void Renderer::UpdatePointLightBuffer() const
{
	struct PointLights_TO_GPU
	{
		unsigned int Count;
		float Constant;
		float Linear;
		float Quadratic;
		struct PerLightInfo
		{
			alignas(16) DirectX::XMFLOAT3 light; //color * intensity
			alignas(16) DirectX::XMFLOAT3 position;
		};
		PerLightInfo lights[4];
	};
	
	static PointLights_TO_GPU toGPU;
	toGPU.Count = PointLight::lights.size();
	toGPU.Constant = PointLight::constant;
	toGPU.Linear = PointLight::linear;
	toGPU.Quadratic = PointLight::quadratic;

	for (unsigned int i = 0; i < PointLight::MaxCount && i < toGPU.Count; i++)
	{
		const PointLight& currentLight = *PointLight::lights[i];
		DirectX::XMFLOAT3 worldPos = currentLight.GetEntity()->GetTransform()->GetWorldPosition();

		//Light calculations in view space
		DirectX::XMVECTOR pos = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&worldPos),Graphics::viewMatrix);
		DirectX::XMStoreFloat3(&toGPU.lights[i].position,pos);
		
		toGPU.lights[i].light = {
			currentLight.color.x * currentLight.intensity,
			currentLight.color.y * currentLight.intensity,
			currentLight.color.z * currentLight.intensity
		};
	}

	//First time initialization
	static PS_ConstantBuffer<PointLights_TO_GPU> pointLightBuffer = {
		&toGPU,
		1u,
		2u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true
	};

	pointLightBuffer.ChangeData(&toGPU);
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