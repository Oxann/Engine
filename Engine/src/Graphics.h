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
public:
	enum class ProjectionType
	{
		Orthographic,
		Perspective
	};
	enum class ClearMode
	{
		Black,
		Grey
	};
public:
	static void Init(HWND hWnd);
	Graphics() = delete;
	Graphics(const Graphics&) = delete;
	Graphics(Graphics&&) = delete;
	Graphics& operator= (const Graphics&) = delete;
	Graphics& operator= (Graphics&&) = delete;
	static void EndFrame();
	static void BeginFrame();
	static void Enable_VSYNC();
	static void Disable_VSYNC();
	static void SetClearMode(ClearMode clearMode);

	//Projection
	static float GetAspectRatio();

	//If the projection type is orthographic then this returns less than 0.
	//Angle in degrees.
	static float GetVerticalFOV();

	//If the projection type is orthographic then this returns less than 0.
	//Angle in degrees.
	static float GetHorizontalFOV();
	
	//Height is in camera space from top to bottom. (NOT HALF!!!)
	//If the projection type is perspective then height is vertical FOV angle.
	static void SetProjection(ProjectionType type, float aspectRatio, float height,float near_z, float far_z);

	static void SetAmbientColor(DirectX::XMFLOAT3 color);
	static void SetAmbientIntensity(float intensity);
	static DirectX::XMFLOAT3 GetAmbientColor();
	static float GetAmbientIntensity();

public:
	//DX objects
	inline static Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11Device> pDevice = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pView = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencil = nullptr;

private:
	inline static bool isVSyncEnabled = false;
	inline static ClearMode clearMode;
	inline static float clearColor[4];
	inline static Resolution renderResolution;

	//Ambient Light
	inline static DirectX::XMVECTOR ambientLight; // w is intensity
	inline static PS_ConstantBuffer<DirectX::XMVECTOR>* ambientLightBuffer;	

	//Currently if we are in editor mode, view matrix is editor camera's lookat matrix
	inline static DirectX::XMMATRIX* viewMatrix;

	//Projection stuff
	inline static DirectX::XMMATRIX projectionMatrix;
	inline static float verticalFOV;
	inline static float horizontalFOV;
	inline static float aspectRatio;
	inline static ProjectionType projectionType;
};