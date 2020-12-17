#pragma once

#include "ResourceBase.h"
#include "EngineException.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"

#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>

class Resources
{
	friend class MainWindow;
	friend Model;
	friend class EditorResourceSelectionWindow;
public:
	static Texture* FindTexture(std::string file)
	{
		const auto& texture = Textures.find(file);
		
		if (texture == Textures.end())
			return nullptr;
		else
			return texture->second.get();
	}
	static VertexShader* FindVertexShader(std::string file)
	{
		const auto& vs = VertexShaders.find(file);

		if (vs == VertexShaders.end())
			return nullptr;
		else
			return vs->second.get();
	}
	static PixelShader* FindPixelShader(std::string file)
	{
		const auto& ps = PixelShaders.find(file);

		if (ps == PixelShaders.end())
			return nullptr;
		else
			return ps->second.get();
	}
	static Model* FindModel(std::string file)
	{
		const auto& model = Models.find(file);

		if (model == Models.end())
			return nullptr;
		else
			return model->second.get();
	}
private:
	static void Init();
private:
	static inline std::unordered_map<std::string, std::unique_ptr<Texture>> Textures;
	static inline std::unordered_map<std::string, std::unique_ptr<VertexShader>> VertexShaders;
	static inline std::unordered_map<std::string, std::unique_ptr<PixelShader>> PixelShaders;
	static inline std::unordered_map<std::string, std::unique_ptr<Model>> Models;
};