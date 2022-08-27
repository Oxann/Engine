#pragma once
#include "Graphics.h"
#include "ShaderView.h"

class Texture;

class Skybox
{
public:
	Skybox(	std::string_view front,
			std::string_view back,
			std::string_view up,
			std::string_view down,
			std::string_view right,
			std::string_view left );

	explicit Skybox(std::string_view equirectangularMap, unsigned int outputResolution = 512u);

	explicit Skybox(Microsoft::WRL::ComPtr<ID3D11Texture2D> textureCube);

	void Draw();

	Microsoft::WRL::ComPtr<ID3D11Texture2D> GetTexture() const;

	//Returns cube map texture.
	static Microsoft::WRL::ComPtr<ID3D11Texture2D> CreateCubemapFromEquirectangularMap(Microsoft::WRL::ComPtr<ID3D11Texture2D> equirectangularMap, unsigned int outputResolution = 512);
private:
	void InitStates();
private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
};