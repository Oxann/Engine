#include "TextureSerializer.h"
#include "Texture.h"
#include "../thirdparty/json.hpp"
#include "EngineAssert.h"
#include <filesystem>
#include <fstream>

void TextureSerializer::Serialize(const Texture* texture)
{
	nlohmann::json json;
	json["isSRGB"] = texture->GetColorSpace() == Texture::ColorSpace::sRGB;
	json["filterMode"] = texture->GetFilterMode();
	json["anisotropy"] = texture->GetAnisotropy();

	std::filesystem::path texturePath = texture->GetPath();
	std::ofstream out(texturePath.replace_extension(fileExtension));
	out << json.dump(4);
}

void TextureSerializer::Deserialize(std::filesystem::path path, Data* data)
{
	std::ifstream in(path.replace_extension(fileExtension));

	if (in)
	{
		try
		{
			nlohmann::json json;
			in >> json;

			data->anisotropy = json["anisotropy"];
			data->filterMode = json["filterMode"];
			data->isSRGB = json["isSRGB"];
		}
		catch (std::exception& e)
		{
			ENGINEASSERT(false, path.string() + " is corrupted: " + e.what())
		}
	}
	else
	{
		nlohmann::json json;
		json["isSRGB"] = false;
		json["filterMode"] = Texture::FilterMode::ANISOTROPIC;
		json["anisotropy"] = 16;
		
		std::ofstream out(path);
		out << json.dump(4);

		*data = {};
	}
}
