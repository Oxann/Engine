#pragma once

#include "ResourceBase.h"
#include "EngineException.h"
#include "Texture.h"
#include "Model.h"
#include "Shader.h"
#include "Material.h"

#include <sstream>
#include <unordered_map>
#include <vector>
#include <memory>

class Resources
{
	friend class Engine;
	friend Model;
	friend class EditorResourceSelectionWindow;
public:
	static Texture* FindTexture(const std::string& file);

	static Shader* FindShader(const std::string& shaderName);

	static Model* FindModel(const std::string& file);

	static Material* FindMaterial(const std::string& file);
private:
	static void Init();
	static void LoadShaders();
	static void LoadTextures(const std::vector<std::filesystem::path>& texturePaths);
	static void InitDefaultMaterials();
	static void InitDefaultTextures();

private:
	static inline std::unordered_map<std::string, std::unique_ptr<Texture>> Textures;
	static inline std::unordered_map<std::string, std::unique_ptr<Shader>> shaders;
	static inline std::unordered_map<std::string, std::unique_ptr<Model>> Models;
	static inline std::unordered_map<std::string, std::unique_ptr<Material>> Materials;

	static inline const std::string shaderDir = "src\\Shaders";
};