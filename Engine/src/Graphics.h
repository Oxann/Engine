#pragma once
#pragma comment(lib, "d3d11.lib")

#include "Win.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>
#include "ConstantBuffer.h"
#include "Light.h"

struct Resolution
{
	int width;
	int height;
};

class Graphics
{
	friend class D3DBase;
	friend class Renderer;
	friend class MainWindow;
	friend class Engine;
	friend class Camera;

#ifdef EDITOR
	friend class Editor;
#endif

public:

	enum class ClearMode
	{
		Black,
		Grey
	};
public:
	static void Init(HWND hWnd);

	static const Resolution& GetResolution();

	//Projection
	static float GetAspectRatio();
	
	//Height is in camera space from top to bottom. (NOT HALF!!!)
	//If the projection type is perspective then height is vertical FOV angle.

	static void SetAmbientColor(DirectX::XMFLOAT3 color);
	static void SetAmbientIntensity(float intensity);
	static DirectX::XMFLOAT3 GetAmbientColor();
	static float GetAmbientIntensity();
	static const Resolution& GetRenderResolution() { return renderResolution; }

	static const DirectX::XMMATRIX& GetViewMatrix() { return viewMatrix; }
	static const DirectX::XMMATRIX& GetProjectionMatrix() { return projectionMatrix; }
public:
	//DX objects
	inline static Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11Device> pDevice = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pView = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencil = nullptr;
	inline static D3D11_VIEWPORT viewport;

	inline static float clearColor[4] = {0.22f, 0.22f, 0.22f, 1.0f};
	inline static bool isVSyncEnabled = false;

private:	
	inline static Resolution renderResolution;

	//Ambient Light
	inline static DirectX::XMVECTOR ambientLight; // w is intensity
	inline static PS_ConstantBuffer<DirectX::XMVECTOR>* ambientLightBuffer;	

	//Projection stuff
	inline static DirectX::XMMATRIX projectionMatrix;
	inline static float aspectRatio;

	//Currently if we are in editor mode, view matrix is editor camera's lookat matrix
	inline static DirectX::XMMATRIX viewMatrix = DirectX::XMMatrixIdentity();
};