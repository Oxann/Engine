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
	static Texture* FindTexture(std::string file);

	static VertexShader* FindVertexShader(std::string file);

	static PixelShader* FindPixelShader(std::string file);

	static Model* FindModel(std::string file);

private:
	static void Init();
private:
	static inline std::unordered_map<std::string, std::unique_ptr<Texture>> Textures;
	static inline std::unordered_map<std::string, std::unique_ptr<VertexShader>> VertexShaders;
	static inline std::unordered_map<std::string, std::unique_ptr<PixelShader>> PixelShaders;
	static inline std::unordered_map<std::string, std::unique_ptr<Model>> Models;
};