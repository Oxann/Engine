#include "SkinnedRenderer.h"
#include "Transform.h"
#include "Entity.h"
#include "Scene.h"
#include "Input.h"

#include <algorithm>

void SkinnedRenderer::Start()
{
	Scene::GetActiveScene()->rendererManager.skinnedRenderers.push_back(this);
	rendererManager = &Scene::GetActiveScene()->rendererManager;
}

void SkinnedRenderer::PrepareForRendering()
{
	PrepareBoneData();

	unsigned int materialIndex = 0;

	for (int i = 0; i < meshes.size(); i++)
	{
		auto mesh = meshes[i];
		for (int j = 0; j < mesh->GetSubMeshCount(); j++)
		{
			if (materials[j]->mode == Material::Mode::Opaque)
			{
				Scene::GetActiveScene()->rendererManager.renderQueueOpaque.Add(this, (materialIndex << 16) + (i << 8) + j);
			}
			materialIndex++;
		}
	}
}

SkinnedRenderer* SkinnedRenderer::Clone()
{
	SkinnedRenderer* newRenderer = new SkinnedRenderer;
	newRenderer->materials = this->materials;
	newRenderer->meshes = this->meshes;
	newRenderer->skeleton = this->skeleton;
	return newRenderer;
}

bool SkinnedRenderer::AddMesh(Mesh* mesh)
{
	if (std::ranges::find(meshes, mesh) == meshes.end())
	{
		meshes.emplace_back(mesh);
		
		for (int i = 0; i < mesh->GetSubMeshCount(); i++)
			materials.emplace_back(nullptr);

		return true;
	}

	return false;
}

bool SkinnedRenderer::RemoveMesh(Mesh* mesh)
{
	int meshIndex = 0;
	for (; meshIndex < meshes.size(); meshIndex++)
		if (meshes[meshIndex] == mesh)
			break;

	if (meshIndex == meshes.size())
		return false;

	RemoveMesh(meshIndex);

	return true;
}

void SkinnedRenderer::RemoveMesh(unsigned int index)
{
	materials.erase(materials.begin() + index, materials.begin() + index + meshes[index]->GetSubMeshCount());
	meshes.erase(meshes.begin() + index);
}

Mesh* SkinnedRenderer::GetMesh(unsigned int index) const
{
	return meshes[index];
}

unsigned int SkinnedRenderer::GetMeshCount() const
{
	return meshes.size();
}

void SkinnedRenderer::SetBones(Transform* root)
{
	_SetBones(root, 0);
}

const std::vector<SkinnedRenderer::Bone>& SkinnedRenderer::GetBones() const
{
	return bones;
}

void SkinnedRenderer::Render(unsigned int subMeshIndex)
{
	const Mesh::SubMesh& subMesh = meshes[(subMeshIndex & 0x00F0) >> 8]->GetSubMeshes()[subMeshIndex & 0x000F];
	subMesh.GetIndexBuffer()->BindPipeline();

	Material* material = materials[subMeshIndex >> 16];
	material->Bind(&subMesh);

	BindBoneData();

	Graphics::pDeviceContext->DrawIndexed(subMesh.GetIndexCount(), 0u, 0u);
}

void SkinnedRenderer::Render(unsigned int subMeshIndex, const Material* material)
{

}

void SkinnedRenderer::PrepareBoneData()
{
	boneWorldMatrices[0] = bones[0].transform->GetWorldMatrix();

	for (int i = 1; i < bones.size(); i++)
	{
		boneWorldMatrices[i] = bones[i].transform->GetLocalMatrix() * boneWorldMatrices[bones[i].parentIndex];
	}

	for (int i = 0; i < bones.size(); i++)
	{
		boneWorldMatrices[i] = skeleton->GetBoneMatrix(i) * boneWorldMatrices[i];
	}

	for (int i = 0; i < bones.size(); i++)
		boneWorldMatrices[i] = DirectX::XMMatrixTranspose(boneWorldMatrices[i]);
}

void SkinnedRenderer::BindBoneData()
{
	Shader::GetVertexShaderBoneBuffer()->ChangeData((Shader::VertexShaderBoneBuffer*)boneWorldMatrices);
}

void SkinnedRenderer::_SetBones(Transform* bone, unsigned int parentIndex)
{
	bones.emplace_back(bone, parentIndex);

	parentIndex = bones.size() - 1;

	for (int i = 0; i < bone->GetEntity()->GetChildrenCount(); i++)
	{
		_SetBones(bone->GetEntity()->GetChild(i)->GetTransform(), parentIndex);
	}
}
