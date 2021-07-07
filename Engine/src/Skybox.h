#pragma once
#include "Graphics.h"
#include "ShaderView.h"


class Skybox
{
public:
	Skybox(	const std::string& front,
			const std::string& back,
			const std::string& up,
			const std::string& down,
			const std::string& right,
			const std::string& left );

	void Draw();

private:
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> textureSRV;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> depthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizerState;
	ShaderView shader;
};