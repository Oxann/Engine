#include "Resources.h"
#include <filesystem>
#include "EngineException.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"
#include "Engine.h"

using namespace std::filesystem;


void Resources::Init()
{
	ENGINE_LOG(ENGINE_INFO, "Resources loading...");

	auto time1 = std::chrono::steady_clock::now();

	LoadShaders();
	InitDefaultMaterials();

	std::vector<path> texturesToLoad;
	std::vector<path> modelsToLoad;
	texturesToLoad.reserve(100);
	modelsToLoad.reserve(100);

	for (auto& file : recursive_directory_iterator("Resources"))
	{
		if (!file.is_directory())
		{
			std::string extension = file.path().extension().string();

			if (extension == ".png" || extension == ".jpg")
			{
				texturesToLoad.emplace_back(file.path());
				//Textures.insert({ file.path().string(),std::make_unique<Texture>(file.path()) });
			}
			else if (extension == ".obj" || extension == ".fbx" || extension == ".max" || extension == ".FBX")
			{
				modelsToLoad.emplace_back(file.path());
				//Models.insert({ file.path().string(),std::make_unique<Model>(file.path()) });
			}
		}
	}

	LoadTextures(texturesToLoad);

	for (auto& modelToLoad : modelsToLoad)
	{
		Models.emplace(std::piecewise_construct, std::forward_as_tuple(modelToLoad.string()), std::forward_as_tuple(std::make_unique<Model>(modelToLoad)));
	}

	auto time2 = std::chrono::steady_clock::now();
	ENGINE_LOG(ENGINE_INFO, std::to_string(std::chrono::duration<float>(time2 - time1).count()))

	ENGINE_LOG(ENGINE_INFO, "Resources successfully loaded.");
}

void Resources::LoadShaders()
{
	auto shadersLoadBegin = std::chrono::steady_clock::now();

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

	auto shadersLoadEnd = std::chrono::steady_clock::now();
	ENGINE_LOG(ENGINE_INFO, "Shaders loaded in " + std::to_string(std::chrono::duration<float>(shadersLoadEnd - shadersLoadBegin).count()) + " seconds.")
}

void Resources::LoadTextures(const std::vector<path>& texturePaths)
{
	std::mutex textureLoadMutex;
	std::vector<std::future<void>> textureLoadHandles;

	auto texturesLoadBegin = std::chrono::steady_clock::now();

	for (int i = 0; i < texturePaths.size(); i++)
	{
		textureLoadHandles.push_back(Engine::threadPool.AddTask<void>([&texturePath = texturePaths[i], &textureLoadMutex]{
			auto newTexture = std::make_unique<Texture>(texturePath);

			textureLoadMutex.lock();
			Textures.emplace(std::piecewise_construct, std::forward_as_tuple(texturePath.string()), std::forward_as_tuple(std::move(newTexture)));
			textureLoadMutex.unlock();
			}));
	}

	for (auto& textureLoadHandle : textureLoadHandles)
		textureLoadHandle.get();

	auto texturesLoadEnd = std::chrono::steady_clock::now();
	ENGINE_LOG(ENGINE_INFO, "Textures loaded in " + std::to_string(std::chrono::duration<float>(texturesLoadEnd - texturesLoadBegin).count()) + " seconds.")
}

void Resources::InitDefaultMaterials()
{
	Shader::GetVertexShaderPerFrameBuffer();
	Shader::GetVertexShaderPerObjectBuffer();

	Shader* unlitShader = FindShader("Unlit");
	Shader* litShader = FindShader("Lit");

	Material* wireframe = Materials.insert({ "$Default\\Wireframe", std::make_unique<Material>("Wireframe", unlitShader) }).first->second.get();
	wireframe->SetFloat4("color", { 0.0f, 0.6f, 0.0f, 1.0f });

	auto x = wireframe->GetFloat4("color");

	Material* outline = Materials.insert({ "$Default\\Outline", std::make_unique<Material>("Outline", unlitShader) }).first->second.get();
	outline->SetFloat4("color", { 1.0f, 0.0f, 0.0f, 1.0f });

	Material* lit = Materials.insert({ "$Default\\Lit", std::make_unique<Material>("Lit", litShader) }).first->second.get();
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
