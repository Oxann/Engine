#pragma once
#pragma comment(lib, "d3d11.lib")

#include "Win.h"
#include <d3d11.h>
#include <wrl.h>
#include <vector>
#include <DirectXMath.h>
#include "ConstantBuffer.h"
#include "Light.h"
#include "Unlit_Material.h"

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

	static const Unlit_Material& GetWireframeMaterial();

	static const DirectX::XMMATRIX& GetViewMatrix() { return *viewMatrix; }
	static const DirectX::XMMATRIX& GetProjectionMatrix() { return projectionMatrix; }
	static const DirectX::XMMATRIX& GetViewProjectionMatrix() { return viewProjectionMatrix; }
private:
	static void InitRS();
	static void InitBS();
public:
	//DX objects
	inline static Microsoft::WRL::ComPtr<IDXGISwapChain> pSwapChain = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11Device> pDevice = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DeviceContext> pDeviceContext = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pView = nullptr;
	inline static Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencil = nullptr;

	//Rasterizer States
	inline static Microsoft::WRL::ComPtr<ID3D11RasterizerState> RS_Solid;
	inline static Microsoft::WRL::ComPtr<ID3D11RasterizerState> RS_Wireframe;

	//Blend States
	inline static Microsoft::WRL::ComPtr<ID3D11BlendState> BS_Opaque;
	inline static Microsoft::WRL::ComPtr<ID3D11BlendState> BS_Transparent;
private:
	inline static bool isVSyncEnabled = false;
	inline static ClearMode clearMode;
	inline static float clearColor[4];
	inline static Resolution renderResolution;

	//Ambient Light
	inline static DirectX::XMVECTOR ambientLight; // w is intensity
	inline static PS_ConstantBuffer<DirectX::XMVECTOR>* ambientLightBuffer;	

	//Projection stuff
	inline static DirectX::XMMATRIX projectionMatrix;
	inline static float verticalFOV;
	inline static float horizontalFOV;
	inline static float aspectRatio;
	inline static ProjectionType projectionType;

	inline static DirectX::XMMATRIX viewProjectionMatrix;

	//Currently if we are in editor mode, view matrix is editor camera's lookat matrix
	inline static DirectX::XMMATRIX* viewMatrix;
};