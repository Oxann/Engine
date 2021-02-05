#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <DirectXMath.h>

#include "Mesh.h"
#include "Material.h"
#include "ResourceBase.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


class Model : public ResourceBase
{
private:
	class Node
	{
		friend Model;
	public:
		Node(DirectX::XMMATRIX localMatrix)
			:localMatrix(localMatrix)
		{}
		const Mesh* GetMesh() const
		{
			return mesh;
		}
	public:
		Node* parent = nullptr;
		std::string name;
		std::vector<std::unique_ptr<Node>> children;
		const DirectX::XMMATRIX localMatrix;
	private:
		const Mesh* mesh = nullptr;
		std::vector<Material*> materials;
	};
public:
	Model(std::filesystem::path file);
	size_t GetMeshCount() const;
	const Mesh* FindMesh(std::string name) const;
	const Material* FindMaterial(std::string name) const;
	const std::unordered_map<std::string, std::unique_ptr<Mesh>>& GetMeshes() const;
	const std::unordered_map<std::string, std::unique_ptr<Material>>& GetMaterials() const;
private:
	void InitHierarchy(Node* node, aiNode* ai_node,const aiScene* scene,Entity* newPrefab);
	std::unique_ptr<Node> CloneNode(aiNode* ai_node, const aiScene* scene);

	//If the texture is not exist in the parent folder this returns nullptr.
	Texture* GetTextureInParentFolder(std::filesystem::path textureName) const;
private:
	//All the meshes this model contains.
	std::unordered_map<std::string, std::unique_ptr<Mesh>> meshes;
	std::unordered_map<std::string, std::unique_ptr<Material>> materials;
	std::unique_ptr<Node> root;
};