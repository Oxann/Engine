#include "Renderer.h"
#include "Entity.h"
#include "Graphics.h"
#include "EngineException.h"
#include "Light.h"
#include "EngineAssert.h"
#include "Phong_Material.h"

void Renderer::Start()
{	
	if (mesh == nullptr)
		SetMesh(Mesh::Cube);
	
	transform = GetEntity()->GetTransform();
}

void Renderer::Update()
{
	if (mesh)
	{
		//Updating topology
		Graphics::pDeviceContext->IASetPrimitiveTopology(topology);
		
		//Rendering
		for (int i = 0; i < materials.size() && i < mesh->GetSubMeshCount(); i++)
		{

#ifdef EDITOR
			if (Graphics::isWireframeEnabled)
				Graphics::GetWireframeMaterial().Bind(&mesh->GetSubMeshes()[i], this);
			else
#endif
				materials[i]->Bind(&mesh->GetSubMeshes()[i], this);
			
			Graphics::pDeviceContext->DrawIndexed(mesh->GetSubMeshes()[i].GetIndexCount(), 0u, 0u);
		}
	}
}

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
	materials.reserve(mesh->GetSubMeshCount());

	for (int i = 0; i < mesh->GetSubMeshCount(); i++)
		materials.push_back(Phong_Material::GetDefaultMaterial());
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
			DirectX::XMMatrixRotationQuaternion(currentLight.GetEntity()->GetTransform()->GetWorldQuaternion()), *Graphics::viewMatrix);
		
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
		DirectX::XMVECTOR pos = DirectX::XMVector3Transform(DirectX::XMLoadFloat3(&worldPos),*Graphics::viewMatrix);
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

void Renderer::UpdateTransformBuffer_Only_MVP() const
{
	static DirectX::XMMATRIX MVP;
	MVP = DirectX::XMMatrixTranspose(
				DirectX::XMMatrixMultiply(
					DirectX::XMMatrixMultiply(transform->GetWorldMatrix(), *Graphics::viewMatrix),
					Graphics::projectionMatrix));

	//First time initialization
	static VS_ConstantBuffer<DirectX::XMMATRIX> TransformBuffer = {
		&MVP,
		1u,
		0u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};

	TransformBuffer.ChangeData(&MVP);
	TransformBuffer.BindPipeline();
}

void Renderer::UpdateTransformBuffer() const
{
	//Transform data
	struct TransformData
	{
		DirectX::XMMATRIX modelView;
		DirectX::XMMATRIX normalMatrix;
		DirectX::XMMATRIX MVP;
	};

	static TransformData entityTransformData;
	entityTransformData.modelView = DirectX::XMMatrixMultiply(transform->GetWorldMatrix(),*Graphics::viewMatrix);
	entityTransformData.normalMatrix = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, DirectX::XMMatrixTranspose(entityTransformData.modelView)));

	//Matrices are row order we need to make them column. 
	entityTransformData.MVP = DirectX::XMMatrixTranspose(DirectX::XMMatrixMultiply(entityTransformData.modelView,Graphics::projectionMatrix));
	entityTransformData.modelView = DirectX::XMMatrixTranspose(entityTransformData.modelView);

	//First time initialization
	static VS_ConstantBuffer<TransformData> TransformBuffer = {
		&entityTransformData,
		1u,
		0u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		false
	};
		
	TransformBuffer.ChangeData(&entityTransformData);
	TransformBuffer.BindPipeline();
}

