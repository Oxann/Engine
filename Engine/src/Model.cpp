#pragma once
#include "Model.h"
#include "Scene.h"
#include "Entity.h"
#include "EngineException.h"
#include "Resources.h"
#include "Renderer.h"
#include "SkinnedRenderer.h"

#include <filesystem>

using namespace Assimp;

Model::Model(const std::filesystem::path& file)
	:ResourceBase(file)
{
	Importer importer;

	//Ignoring lights and cameras
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent::aiComponent_CAMERAS | aiComponent::aiComponent_LIGHTS);
	importer.SetPropertyInteger(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
	importer.SetPropertyFloat(AI_CONFIG_PP_FID_ANIM_ACCURACY, 0.0f);
	
	const unsigned int pFlags = aiProcess_FlipUVs |
		aiProcess_MakeLeftHanded |
		aiProcess_FlipWindingOrder |
		aiProcess_JoinIdenticalVertices |
		aiProcess_Triangulate |
		aiProcess_RemoveComponent |
		aiProcess_CalcTangentSpace |
		aiProcess_GenSmoothNormals |
		aiProcess_GenBoundingBoxes |
		aiProcess_PopulateArmatureData |
		aiProcess_FindInvalidData;

	auto scene = importer.ReadFile(file.string(), pFlags);
	
	if (scene == nullptr)
	{
		std::stringstream msg;
		msg << file.string() << " - " << importer.GetErrorString();
		THROW_ENGINE_EXCEPTION(msg.str(), true);
	}

	//Prefab root
	Entity* newPrefab = new Entity(file.filename().string());
	Transform* newPrefabRootBone = nullptr;
	SkinnedRenderer* skinnedRenderer = nullptr;
	aiNode* rootBone = nullptr;

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		if (scene->mMeshes[i]->HasBones())
		{
			skinnedRenderer = newPrefab->AddChild("SkinnedRenderer")->AddComponent<SkinnedRenderer>();
			rootBone = scene->mMeshes[i]->mBones[0]->mNode;
			break;
		}
	}

	root = CloneNode(scene->mRootNode, scene, skinnedRenderer);
	newPrefab->GetTransform()->SetLocalMatrix(root->localMatrix);


 	if (root->GetMesh())
	{
		if (root->hasBones)
		{
			skinnedRenderer->AddMesh(root->GetMesh());
			for (int i = 0; i < root->materials.size(); i++)
			{
				root->materials[i]->ActivateMacro("SKINNING");
				skinnedRenderer->SetMaterial(root->materials[i], i);
			}
		}
		else
		{
			Renderer* renderer = newPrefab->AddComponent<Renderer>();
			renderer->SetMesh(root->GetMesh());
			for (int i = 0; i < root->materials.size(); i++)
				renderer->SetMaterial(root->materials[i], i);
		}
	}
		
	InitHierarchy(root.get(), scene->mRootNode, scene,newPrefab, skinnedRenderer, rootBone, &newPrefabRootBone);
	
	if (newPrefabRootBone)
	{
		skeleton = std::make_shared<Skeleton>(newPrefabRootBone);
		skinnedRenderer->SetBones(newPrefabRootBone);
		skinnedRenderer->skeleton = skeleton;
	}

	LoadAnimations(scene);

	//Creating a new prefab from this model.
	Entity::Prefabs.insert({ newPrefab->name,std::unique_ptr<Entity>(newPrefab) });
}

size_t Model::GetMeshCount() const
{
	return meshes.size();
}

const Mesh* Model::FindMesh(const std::string& name) const
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

const Material* Model::FindMaterial(const std::string& name) const
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

std::shared_ptr<Animation> Model::FindAnimation(std::string_view name) const
{
	for (const auto& animation : animations)
	{
		if (animation->name == name)
			return animation;
	}

	return nullptr;
}

const std::unordered_map<std::string, std::unique_ptr<Mesh>>& Model::GetMeshes() const
{
	return meshes;
}

const std::unordered_map<std::string, std::unique_ptr<Material>>& Model::GetMaterials() const
{
	return materials;
}

void Model::InitHierarchy(Node* node, aiNode* ai_node, const aiScene* scene,Entity* newPrefab, SkinnedRenderer* skinnedRenderer, aiNode* rootBone, Transform** prefabRootBone)
{
	if (ai_node == rootBone)
		*prefabRootBone = newPrefab->GetTransform();

	for (int i = 0; i < ai_node->mNumChildren; i++)
	{
		//Model
		node->children.push_back(CloneNode(ai_node->mChildren[i],scene, skinnedRenderer));
		Node* newChild = node->children.back().get();
		newChild->parent = node;

		//Prefab
		Entity* newPrefab_newChild = newPrefab->AddChild(newChild->name);
		newPrefab_newChild->GetTransform()->SetLocalMatrix(newChild->localMatrix);
		newPrefab_newChild->parent = newPrefab;


		if (newChild->GetMesh())
		{
			if (newChild->hasBones)
			{
				skinnedRenderer->AddMesh(newChild->GetMesh());
				for (int i = 0; i < newChild->materials.size(); i++)
				{
					newChild->materials[i]->ActivateMacro("SKINNING");
					skinnedRenderer->SetMaterial(newChild->materials[i], i);
				}
			}
			else
			{
				Renderer* renderer = newPrefab_newChild->AddComponent<Renderer>();
				renderer->SetMesh(newChild->GetMesh());
				for (int i = 0; i < newChild->materials.size(); i++)
					renderer->SetMaterial(newChild->materials[i], i);
			}
		}
			
		InitHierarchy(newChild, ai_node->mChildren[i], scene,newPrefab_newChild, skinnedRenderer, rootBone, prefabRootBone);
	}
}

std::unique_ptr<Model::Node> Model::CloneNode(aiNode* ai_node, const aiScene* scene, SkinnedRenderer* skinnedRenderer)
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

			//SubMesh AABB
			newSubMesh.AABB.Center = {
				(mesh->mAABB.mMax.x + mesh->mAABB.mMin.x) / 2.0f,
				(mesh->mAABB.mMax.y + mesh->mAABB.mMin.y) / 2.0f,
				(mesh->mAABB.mMax.z + mesh->mAABB.mMin.z) / 2.0f 
			};
			newSubMesh.AABB.Extents = {
				(mesh->mAABB.mMax.x - newSubMesh.AABB.Center.x),
				(mesh->mAABB.mMax.y - newSubMesh.AABB.Center.y),
				(mesh->mAABB.mMax.z - newSubMesh.AABB.Center.z)
			};

			//Updating mesh AABB
			if (i == 0)
				newMesh->AABB = newSubMesh.AABB;				
			else
				DirectX::BoundingBox::CreateMerged(newMesh->AABB, newMesh->AABB, newSubMesh.AABB);
			
			newSubMesh.SetVertexCount(mesh->mNumVertices);

			if (mesh->HasPositions())
			{
				newSubMesh.AddNewVertexElement(mesh->mVertices, VertexBuffer::ElementType::Position3D);
				newSubMesh.positions.resize(mesh->mNumVertices);
				std::copy(reinterpret_cast<DirectX::XMFLOAT3*>(mesh->mVertices), reinterpret_cast<DirectX::XMFLOAT3*>(mesh->mVertices) + mesh->mNumVertices, newSubMesh.positions.begin());
			}

			if (mesh->HasBones())
			{
				newNode->hasBones = true;

				struct BoneIDs
				{
					int ids[4] = {-1, -1, -1, -1};
				};

				struct Weights
				{
					float weights[4] = {0.0f,0.0f,0.0f,0.0f};
				};

				std::vector<BoneIDs> boneIDs(mesh->mNumVertices);
				std::vector<Weights> weights(mesh->mNumVertices);
				
				for (int currentBoneID = 0; currentBoneID < mesh->mNumBones; currentBoneID++)
				{
					auto currentBone = mesh->mBones[currentBoneID];

					for (int j = 0; j < currentBone->mNumWeights; j++)
					{
						int currentVertexID = currentBone->mWeights[j].mVertexId;
						float weight = currentBone->mWeights[j].mWeight;

						bool doesCurrentVertexAlreadyHaveThisBone = false;

						for (int m = 0; m < 4; m++)
						{
							if (currentBoneID == boneIDs[currentVertexID].ids[m])
							{
								doesCurrentVertexAlreadyHaveThisBone = true;
								break;
							}
						}

						if (!doesCurrentVertexAlreadyHaveThisBone)
						{
							int replacementIndex = -1;

							for (int m = 0; m < 4; m++)
							{
								if (weight > weights[currentVertexID].weights[m])
								{
									replacementIndex = m;
									break;
								}
							}

							if (replacementIndex > -1)
							{
								for (int m = 2; m >= replacementIndex; m--)
								{
									boneIDs[currentVertexID].ids[m + 1] = boneIDs[currentVertexID].ids[m];
									weights[currentVertexID].weights[m + 1] = weights[currentVertexID].weights[m];
								}

								boneIDs[currentVertexID].ids[replacementIndex] = currentBoneID;
								weights[currentVertexID].weights[replacementIndex] = weight;
							}
						}
					}
				}

				for (int i = 0; i < mesh->mNumVertices; i++)
				{
					float total = 0.0f;

					if (boneIDs[i].ids[0] == -1)
						int x = 5;

					for (int j = 0; j < 4; j++)
					{
						total += weights[i].weights[j];
					}

					for (int j = 0; j < 4; j++)
					{
						weights[i].weights[j] /= total;
					}
				}

				newSubMesh.AddNewVertexElement(boneIDs.data(), VertexBuffer::ElementType::BoneIDs);
				newSubMesh.AddNewVertexElement(weights.data(), VertexBuffer::ElementType::BoneWeights);
			}

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

			for (size_t i = 0; i < mesh->mNumFaces; i++)
			{
				newSubMesh.indices.push_back(mesh->mFaces[i].mIndices[0]);
				newSubMesh.indices.push_back(mesh->mFaces[i].mIndices[1]);
				newSubMesh.indices.push_back(mesh->mFaces[i].mIndices[2]);
			}
			newSubMesh.SetIndexBuffer(newSubMesh.indices.data(), newSubMesh.indices.size());

			//Initializing Material
			aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
			const auto& isMaterialAlreadyExist = materials.find(mat->GetName().C_Str());

			if (isMaterialAlreadyExist == materials.end())
			{	
				static Shader* litShader = Resources::FindShader("LitPBR");
				std::unique_ptr<Material> newMaterial = std::make_unique<Material>(mat->GetName().C_Str(), litShader);

				////////// DIFFUSE //////////
				aiColor4D diffuseColor;
				if(mat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat4("diffuseColor", {diffuseColor.r, diffuseColor.g, diffuseColor.b, diffuseColor.a});

				//if the material has diffuse map we set it
				if (mat->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE))
				{
					newMaterial->ActivateMacro("DIFFUSETEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE, 0u, &temp);
					newMaterial->SetTexture("diffuseTexture", GetTextureInParentFolder(temp.C_Str()));
				}
				/////////////////////////////

				////////// METALNESS //////////
				float metalness;
				
				if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metalness) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat("metalness", metalness);
				else
					newMaterial->SetFloat("metalness", 0.0f);
				
				if (mat->GetTextureCount(aiTextureType::aiTextureType_METALNESS))
				{
					newMaterial->ActivateMacro("METALNESSTEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_METALNESS, 0u, &temp);
					newMaterial->SetTexture("metalnessTexture", GetTextureInParentFolder(temp.C_Str()));
				}
				/////////////////////////////

				////////// SMOOTHNESS //////////
				float roughness;

				if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat("smoothness", 1.0f - roughness);
				else 
					newMaterial->SetFloat("smoothness", 0.5f);

				if (mat->GetTextureCount(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS))
				{
					newMaterial->ActivateMacro("ROUGHNESSTEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_DIFFUSE_ROUGHNESS, 0u, &temp);
					newMaterial->SetTexture("roughnessTexture", GetTextureInParentFolder(temp.C_Str()));
				}
				/////////////////////////////

				////////// AMBIENT OCCLUSION //////////
				if (mat->GetTextureCount(aiTextureType::aiTextureType_AMBIENT_OCCLUSION))
				{
					newMaterial->ActivateMacro("AMBIENTOCCLUSIONTEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_AMBIENT_OCCLUSION, 0u, &temp);
					newMaterial->SetTexture("ambientOcclusionTexture", GetTextureInParentFolder(temp.C_Str()));
				}
				/////////////////////////////

				/*////////// SPECULAR //////////
				aiColor4D specularColor;
				if(mat->Get(AI_MATKEY_COLOR_SPECULAR, specularColor) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat4("matSpecularColor", {specularColor.r, specularColor.g, specularColor.b, specularColor.a});

				float shininess;
				if(mat->Get(AI_MATKEY_SHININESS, shininess) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat("matShininess", std::log2f(shininess) / 13.0f);

				float shininessStrength;				
				if(mat->Get(AI_MATKEY_SHININESS_STRENGTH, shininessStrength) == aiReturn::aiReturn_SUCCESS)
					newMaterial->SetFloat("matShininessStrength", shininessStrength);

				//if the material has speculer map we set it
				if (mat->GetTextureCount(aiTextureType::aiTextureType_SPECULAR))
				{
					newMaterial->ActivateMacro("SPECULARTEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_SPECULAR, 0u, &temp);
					newMaterial->SetTexture("specularTexture", GetTextureInParentFolder(temp.C_Str()));
				}
				//////////////////////////////*/

				//Normal map
				if (mat->GetTextureCount(aiTextureType::aiTextureType_NORMALS))
				{
					newMaterial->ActivateMacro("NORMALTEXTURE");

					aiString temp;
					mat->GetTexture(aiTextureType::aiTextureType_NORMALS, 0u, &temp);
					newMaterial->SetTexture("normalTexture", GetTextureInParentFolder(temp.C_Str()));
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

void Model::LoadAnimations(const aiScene* scene)
{
	if (skeleton && scene->HasAnimations())
	{
		for (int i = 0; i < scene->mNumAnimations; i++)
		{
			auto animation = scene->mAnimations[i];
			animations.emplace_back(std::make_shared<Animation>(skeleton, animation->mDuration / animation->mTicksPerSecond));
			animations.back()->name = animation->mName.C_Str();

			for (int j = 0; j < animation->mNumChannels; j++)
			{
				auto channel = animation->mChannels[j];
				auto node = animations.back()->AddNewNodeFromSkeleton(channel->mNodeName.C_Str());
				
				if (node)
				{
					for (int k = 0; k < channel->mNumPositionKeys; k++)
					{
						auto position = channel->mPositionKeys[k];
						node->positions.emplace_back(DirectX::XMVectorSet(position.mValue.x, position.mValue.y, position.mValue.z, 1.0f), position.mTime / animation->mTicksPerSecond);
					}

					for (int k = 0; k < channel->mNumRotationKeys; k++)
					{
						auto rotation = channel->mRotationKeys[k];
						node->rotations.emplace_back(DirectX::XMVectorSet(rotation.mValue.x, rotation.mValue.y, rotation.mValue.z, rotation.mValue.w), rotation.mTime / animation->mTicksPerSecond);
					}
				
					for (int k = 0; k < channel->mNumScalingKeys; k++)
					{
						auto scale = channel->mScalingKeys[k];
						node->scales.emplace_back(DirectX::XMVectorSet(scale.mValue.x, scale.mValue.y, scale.mValue.z, 1.0f), scale.mTime / animation->mTicksPerSecond);
					}
				}
			}
		}
	}
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
		}
	}

	//If the texture is not found in model's parent folder or sub folders, we return nullptr and log warning.
	std::stringstream ss;
	ss << "Model " << GetPath() << " missing texture."
		<< "Texture Name: " << textureName;
	ENGINE_LOG(ENGINE_WARNING, ss.str());
	
	return nullptr;
}
