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

#if defined NDEBUG && defined EDITOR
	shaderDir = "Shaders\\Release-Editor\\";
#elif defined NDEBUG && !defined EDITOR
	shaderDir = "Shaders\\Release\\";
#elif !defined NDEBUG && defined EDITOR
	shaderDir = "Shaders\\Debug-Editor\\";
#else
	shaderDir = "Shaders\\Debug\\";
#endif

	for (auto& file : recursive_directory_iterator(shaderDir))
	{
		std::string extension = file.path().extension().string();

		if (!file.is_directory())
		{
			if (extension == ".cso")
			{
				std::string fileName = file.path().filename().string();
				std::string shaderType = { fileName[0],fileName[1] };

				if (shaderType == "VS")
				{
					VertexShaders.insert({ file.path().string(),std::make_unique<VertexShader>(file.path()) });
				}
				else if (shaderType == "PS")
				{
					PixelShaders.insert({ file.path().string(),std::make_unique<PixelShader>(file.path()) });
				}
				else
				{
					std::stringstream msg;
					msg << "Shader name is incorrect: " << file.path().string()
						<< "\nPixel shaders must start with PS_ and vertex shaders must start with VS_";
					THROW_ENGINE_EXCEPTION(msg.str(), true);
				}
			}
			else
			{
				std::stringstream msg;
				msg << "Resources\\Shaders can only contain precompiled shaders."
					<< file.path().string() << "\n";					
				THROW_ENGINE_EXCEPTION(msg.str(), true);
			}
		}
	}

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

Texture* Resources::FindTexture(std::string file)
{
	const auto& texture = Textures.find(file);

	if (texture == Textures.end())
		return nullptr;
	else
		return texture->second.get();
}

VertexShader* Resources::FindVertexShader(std::string file)
{
	const auto& vs = VertexShaders.find(shaderDir + "VS_" + file);

	if (vs == VertexShaders.end())
		return nullptr;
	else
		return vs->second.get();
}

PixelShader* Resources::FindPixelShader(std::string file)
{
	const auto& ps = PixelShaders.find(shaderDir + "PS_" + file);

	if (ps == PixelShaders.end())
		return nullptr;
	else
		return ps->second.get();
}

Model* Resources::FindModel(std::string file)
{
	const auto& model = Models.find(file);

	if (model == Models.end())
		return nullptr;
	else
		return model->second.get();
}

Material* Resources::FindMaterial(std::string file)
{
	const auto& material = Materials.find(file);

	if (material == Materials.end())
		return nullptr;
	else
		return material->second.get();
}
