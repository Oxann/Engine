#pragma once

#include "RendererBase.h"
#include "Skeleton.h"

class SkinnedRenderer : public RendererBase
{
	friend RendererManager;
public:
	struct Bone
	{
		Transform* transform;
		unsigned int parentIndex;
	};
public:
	void Start() override;
	void PrepareForRendering() override;
	SkinnedRenderer* Clone() override;
	bool AddMesh(Mesh* mesh);
	bool RemoveMesh(Mesh* mesh);
	void RemoveMesh(unsigned int index);
	Mesh* GetMesh(unsigned int index) const;
	unsigned int GetMeshCount() const;
	void SetBones(Transform* root);
	const std::vector<Bone>& GetBones() const;

	//This functions are used by Render Queues. DON'T CALL FROM ELSEWEHERE.
	//Binded material will be used.
	void Render(unsigned int subMeshIndex) override;

	//Custom material needed.
	void Render(unsigned int subMeshIndex, const Material* material) override;

private:
	void PrepareBoneData();
	void BindBoneData();
	void _SetBones(Transform* bone, unsigned int parentIndex);
public:
	std::shared_ptr<Skeleton> skeleton;
private:
	std::vector<Mesh*> meshes;
	std::vector<Bone> bones;
	DirectX::XMMATRIX boneWorldMatrices[80];
};