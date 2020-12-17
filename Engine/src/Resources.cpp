#include "Resources.h"
#include <filesystem>
#include "EngineException.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"

using namespace std::filesystem;


void Resources::Init()
{
	ENGINE_LOG(ENGINE_INFO, "Resources loading...");

	#ifndef NDEBUG
	std::string shaderDir = "Shaders\\Debug";
	#else
	std::string shaderDir = "Shaders\\Release";
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
			else if (extension == ".obj" || extension == ".fbx" || extension == ".max")
			{
				Models.insert({ file.path().string(),std::make_unique<Model>(file.path()) });
			}
		}
	}

	ENGINE_LOG(ENGINE_INFO, "Resources successfully loaded.");
}
