#include "Resources.h"
#include <filesystem>
#include "EngineException.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"
#include "Unlit_Material.h"

using namespace std::filesystem;


void Resources::Init()
{
	ENGINE_LOG(ENGINE_INFO, "Resources loading...");

	LoadShaders();

	for (auto& file : recursive_directory_iterator("Resources"))
	{
		if (!file.is_directory())
		{
			std::string extension = file.path().extension().string();

			if (extension == ".png" || extension == ".jpg")
			{
				Textures.insert({ file.path().string(),std::make_unique<Texture>(file.path()) });
			}
			else if (extension == ".obj" || extension == ".fbx" || extension == ".max" || extension == ".FBX")
			{
				Models.insert({ file.path().string(),std::make_unique<Model>(file.path()) });
			}
		}
	}


	//Some default resources
#ifdef EDITOR
	Material* wireframe = Materials.insert({ "$Default\\wireframeMaterial", std::make_unique<Unlit_Material>("$Default\\wireframeMaterial") }).first->second.get();
	static_cast<Unlit_Material*>(wireframe)->SetColor({ 0.0f, 0.6f, 0.0f, 1.0f });

	Material* outline  = Materials.insert({ "$Default\\outlineMaterial", std::make_unique<Unlit_Material>("$Default\\outlineMaterial") }).first->second.get();
	static_cast<Unlit_Material*>(outline)->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
#endif

	ENGINE_LOG(ENGINE_INFO, "Resources successfully loaded.");
}

void Resources::LoadShaders()
{
	for (auto& file : recursive_directory_iterator(shaderDir))
	{
		if (!file.is_directory() && file.path().extension() != ".hlsli")
		{
			const std::string shaderName = file.path().filename().replace_extension().string();

			path vsPath = file.path();
			vsPath.replace_extension(".vs");

			path psPath = file.path();
			psPath.replace_extension(".ps");

			const auto& shader = shaders.emplace(shaderName,
				std::make_unique<Shader>(shaderName, vsPath, psPath));
		}
	}
}

Texture* Resources::FindTexture(const std::string& file)
{
	const auto& texture = Textures.find(file);

	if (texture == Textures.end())
		return nullptr;
	else
		return texture->second.get();
}

Shader* Resources::FindShader(const std::string& shaderName)
{
	const auto& shader = shaders.find(shaderName);

	if (shader == shaders.end())
		return nullptr;
	else
		return shader->second.get();
}

Model* Resources::FindModel(const std::string& file)
{
	const auto& model = Models.find(file);

	if (model == Models.end())
		return nullptr;
	else
		return model->second.get();
}

Material* Resources::FindMaterial(const std::string& file)
{
	const auto& material = Materials.find(file);

	if (material == Materials.end())
		return nullptr;
	else
		return material->second.get();
}
