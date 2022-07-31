#pragma once
#include <string>
#include "Texture.h"

class TextureSerializer
{
public:
	struct Data
	{
		Texture::FilterMode filterMode = Texture::FilterMode::ANISOTROPIC;
		int anisotropy = 16;
		bool isSRGB = false;
	};
public:
	static void Serialize(const Texture* texture);
	static void Deserialize(std::filesystem::path path, Data* data);
public:
	inline static std::string fileExtension = "texture";
};