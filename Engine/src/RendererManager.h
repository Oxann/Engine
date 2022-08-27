#pragma once
#include "Renderer.h"
#include "RenderQueueOpaque.h"
#include "RenderQueueTransparent.h"
#include "RenderQueueWireframe.h"
#include "RenderQueueOutline.h"
#include "Camera.h"
#include "Skybox.h"
#include <vector>


class RendererManager
{
	friend class Engine;
	friend class Scene;
	friend Renderer;
	friend class EditorEntityWindow;
	friend class Entity;
	friend PointLight;
	friend DirectionalLight;
private:
	RendererManager();
	void Update();
	void UpdateShadowMaps();
	void UpdateVertexShaderPerFrameBuffer();
	void UpdatePixelShaderPerFrameBuffer();
	void UpdateDirectionalLights();
	void UpdatePointLights();
	void Tonemap();
public:
	unsigned long long meshCount;
	unsigned long long vertexCount;
	unsigned long long triangleCount;

	Camera* activeCamera = nullptr;
private:	
	std::vector<Renderer*> renderers;
	std::vector<DirectionalLight*> directionalLights;
	std::vector<PointLight*> pointLights;

	//Render Queues
	RenderQueueOpaque renderQueueOpaque;
	RenderQueueTransparent renderQueueTransparent;
	RenderQueueWireframe renderQueueWireframe;
	RenderQueueOutline renderQueueOutline;

	//Frustum
	DirectX::BoundingFrustum frustum;

	//Skybox
	Skybox* skybox = nullptr;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> postProcessInputTexture;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> postProcessSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> postProcessRasterizerState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> postProcessBlendState;

private:
	struct DirectionalLights_TO_GPU
	{
		inline static unsigned int slot = 1u;
		inline static unsigned int maxCount = 4u;

		alignas(16) unsigned int Count;
		
		struct PerLightInfo
		{
			float depthBias;
			DirectX::XMFLOAT3 light; //color * intensity
			alignas(16) DirectX::XMFLOAT3 direction;
			int shadowType;
		};
		PerLightInfo lights[4];
	} directionalLights_TO_GPU;

	PS_ConstantBuffer<DirectionalLights_TO_GPU> directionalLightsBuffer = {
		nullptr,
		1u,
		1u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true
	};


	struct PointLights_TO_GPU
	{
		unsigned int Count;
		float Constant;
		float Linear;
		float Quadratic;
		struct PerLightInfo
		{
			float range;
			DirectX::XMFLOAT3 light; //color * intensity
			alignas(16) DirectX::XMFLOAT3 position;
		};
		PerLightInfo lights[4];
	} pointLights_TO_GPU;

	PS_ConstantBuffer<PointLights_TO_GPU> pointLightsBuffer = {
		&pointLights_TO_GPU,
		1u,
		2u,
		D3D11_USAGE::D3D11_USAGE_DYNAMIC,
		D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,
		true
	};
};
