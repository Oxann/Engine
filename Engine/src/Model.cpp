#pragma once
#include "Model.h"
#include "Scene.h"
#include "EngineException.h"
#include "Resources.h"
#include "Phong_Material.h"

#include <filesystem>

using namespace Assimp;

Model::Model(std::filesystem::path file)
	:ResourceBase(file)
{
	Importer importer;

	//Ignoring lights and cameras
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent::aiComponent_CAMERAS | aiComponent::aiComponent_LIGHTS);
	
	const unsigned int pFlags = aiProcess_FlipUVs |
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_RemoveComponent |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals;
	
	auto scene = importer.ReadFile(file.string(), pFlags);
	
	if (scene == nullptr)
	{
		std::stringstream msg;
		msg << file.string() << " - " << importer.GetErrorString();
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}

	root = CloneNode(scene->mRootNode, scene);

	//Prefab root
	Entity* newPrefab = new Entity(file.filename().string());
	newPrefab->GetTransform()->SetLocalMatrix(root->localMatrix);

	if (root->GetMesh() != nullptr)
	{
		Renderer* renderer = newPrefab->AddComponent<Renderer>();
		renderer->SetMesh(root->GetMesh());
		for (int i = 0; i < root->materials.size(); i++)
			renderer->SetMaterial(root->materials[i], i);
	}
		
	InitHierarchy(root.get(), scene->mRootNode, scene,newPrefab);
	
	//Creating a new prefab from this model.
	Entity::MakePrefab(newPrefab);
	delete newPrefab;
}

size_t Model::GetMeshCount() const
{
	return meshes.size();
}

const Mesh* Model::FindMesh(std::string name) const
{
	auto result = meshes.find(name);
	if (result == meshes.end())
	{
		std::stringstream msg;
		msg << GetFileName() << " has no mesh with name " << name;
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}
	return result->second.get();
}

const Material* Model::FindMaterial(std::string name) const
{
	auto result = materials.find(name);
	if (result == materials.end())
	{
		std::stringstream msg;
		msg << GetFileName() << " has no material with name " << name;
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}
	return result->second.get();
}

const std::unordered_map<std::string, std::unique_ptr<Mesh>>& Model::GetMeshes() const
{
	return meshes;
}

const std::unordered_map<std::string, std::unique_ptr<Material>>& Model::GetMaterials() const
{
	return materials;
}

void Model::InitHierarchy(Node* node, aiNode* ai_node, const aiScene* scene,Entity* newPrefab)
{
	for (int i = 0; i < ai_node->mNumChildren; i++)
	{
		//Model
		node->children.push_back(CloneNode(ai_node->mChildren[i],scene));
		Node* newChild = node->children.back().get();
		newChild->parent = node;
		
		//Prefab
		Entity* newPrefab_newChild = newPrefab->AddChild(newChild->name);
		newPrefab_newChild->GetTransform()->SetLocalMatrix(newChild->localMatrix);
		newPrefab_newChild->parent = newPrefab;
		if (newChild->GetMesh() != nullptr)
		{
			Renderer* renderer = newPrefab_newChild->AddComponent<Renderer>();
			renderer->SetMesh(newChild->GetMesh());
			for (int i = 0; i < newChild->materials.size(); i++)
				renderer->SetMaterial(newChild->materials[i],i);
		}
			
		InitHierarchy(newChild, ai_node->mChildren[i], scene,newPrefab_newChild);
	}
}

std::unique_ptr<Model::Node> Model::CloneNode(aiNode* ai_node, const aiScene* scene)
{
	//Initializing node
	std::unique_ptr<Node> newNode = std::make_unique<Node>(
		DirectX::XMMatrixTranspose(DirectX::XMLoadFloat4x4(reinterpret_cast<DirectX::XMFLOAT4X4*>(&ai_node->mTransformation))));
	newNode->name = ai_node->mName.C_Str();
	
	//Initializing meshes
	if (ai_node->mNumMeshes > 0)
	{
		std::unique_ptr<Mesh> newMesh = std::make_unique<Mesh>();
		newMesh->file = newNode->name;
		
		for (int i = 0; i < ai_node->mNumMeshes; i++)
		{			
			aiMesh* mesh = scene->mMeshes[ai_node->mMeshes[i]];
			Mesh::SubMesh& newSubMesh = newMesh->AddNewSubMesh();
			
			newSubMesh.SetVertexCount(mesh->mNumVertices);

			if (mesh->HasPositions())
				newSubMesh.AddNewVertexElement(mesh->mVertices, VertexBuffer::ElementType::Position3D);
			if (mesh->HasNormals())
				newSubMesh.AddNewVertexElement(mesh->mNormals, VertexBuffer::ElementType::Normal);
			if (mesh->HasTextureCoords(0u))
			{
				std::vector<DirectX::XMFLOAT2> uv;
				for (int i = 0; i < mesh->mNumVertices; i++)
				{
					uv.emplace_back(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				}
				newSubMesh.AddNewVertexElement(uv.data(), VertexBuffer::ElementType::TexCoord);
			}
			if (mesh->HasTangentsAndBitangents())
			{
				newSubMesh.AddNewVertexElement(mesh->mTangents, VertexBuffer::ElementType::Tangent);
				newSubMesh.AddNewVertexElement(mesh->mBitangents, VertexBuffer::ElementType::Bitangent);
			}

			std::vector<unsigned short> indices;
			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				indices.push_back(mesh->mFaces[i].mIndices[0]);
				indices.push_back(mesh->mFaces[i].mIndices[1]);
				indices.push_back(mesh->mFaces[i].mIndices[2]);
			}
			newSubMesh.SetIndexBuffer(indices.data(), indices.size());

			//Initializing Material
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			const auto& isMaterialAlreadyExist = materials.find(mat->GetName().C_Str());

			if (isMaterialAlreadyExist == materials.end())
			{				
				std::unique_ptr<Phong_Material> newMaterial = std::make_unique<Phong_Material>(mat->GetName().C_Str());
				
				////////// DIFFUSE //////////
				aiColor4D diffuseColor;
				if(mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetDiffuseColor({ diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a });

				//if the material has diffuse map we set it
				if (mat->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE))
				{
					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0u, &temp);
					newMaterial->SetDiffuseMap(GetTextureInParentFolder(temp.C_Str()));
				}
				/////////////////////////////

				////////// SPECULAR //////////
				aiColor4D specularColor;
				if(mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetSpecularColor({ specularColor.r, specularColor.g, specularColor.b, specularColor.a });

				float shininess;
				if(mat->Get(AI_MATKEY_SHININESS, shininess) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetShininess(std::log2f(shininess) / 13.0f);

				float shininessStrength;				
				if(mat->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetShininessStrength(shininessStrength);

				//if the material has speculer map we set it
				if (mat->GetTextureCount(aiTextureType::aiTextureType_SPECULAR))
				{
					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0u, &temp);
					newMaterial->SetSpecularMap(GetTextureInParentFolder(temp.C_Str()));
				}
				//////////////////////////////

				//Normal map
				if (mat->GetTextureCount(aiTextureType::aiTextureType_NORMALS))
				{
					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_NORMALS, 0u, &temp);
					newMaterial->SetNormalMap(GetTextureInParentFolder(temp.C_Str()));
				}

				const auto& addNewMaterial = materials.insert({ mat->GetName().C_Str(),std::move(newMaterial) });
				newNode->materials.push_back(addNewMaterial.first->second.get());
			}
			else
				newNode->materials.push_back(isMaterialAlreadyExist->second.get());
		}

		newNode->mesh = newMesh.get();
		meshes.insert({ newNode->name,std::move(newMesh) });
	}

	return newNode;
}

Texture* Model::GetTextureInParentFolder(std::filesystem::path textureName) const
{
	textureName = textureName.filename();

	for (auto& file : std::filesystem::recursive_directory_iterator(GetPath().parent_path()))
	{
		if (file.path().filename() == textureName)
		{
			Texture* texture = Resources::FindTexture(file.path().string());
			if (texture)
				return texture;
			else
			{
				auto newTexture = Resources::Textures.insert({ file.path().string(),std::make_unique<Texture>(file.path()) });
				return static_cast<Texture*>(newTexture.first->second.get());
			}
		}
	}

	//If the texture is not found in model's parent folder or sub folders, we return nullptr and log warning.
	std::stringstream ss;
	ss << "Model " << GetPath() << " missing texture."
		<< "Texture Name: " << textureName;
	ENGINE_LOG(ENGINE_WARNING, ss.str());
	
	return nullptr;
}
