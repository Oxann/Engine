#include "Graphics.h"
#include "EngineAssert.h"
#include "Log.h"
#include "MainWindow.h"
#include "EngineException.h"
#include "Shader.h"
#include <DirectXMath.h>
#include "Time.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "Texture.h"
#include "Editor.h"


void Graphics::Init(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd;
	sd.BufferDesc.Width = MainWindow::GetDisplayResolution().width;
	sd.BufferDesc.Height = MainWindow::GetDisplayResolution().height;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;


	UINT flags = 0u;
#ifndef NDEBUG
	flags = D3D11_CREATE_DEVICE_DEBUG;
#endif
	CHECK_DX_ERROR(D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&pSwapChain,
		&pDevice,
		nullptr,
		&pDeviceContext
	));

	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//Viewport
	D3D11_VIEWPORT vp;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	vp.Width = (float)MainWindow::GetDisplayResolution().width;
	vp.Height = (float)MainWindow::GetDisplayResolution().height;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	pDeviceContext->RSSetViewports(1, &vp);

	//Depth and stencil buffer
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> dsState;
	pDevice->CreateDepthStencilState(&dsDesc, &dsState);
	pDeviceContext->OMSetDepthStencilState(dsState.Get(), 0u);

	Microsoft::WRL::ComPtr<ID3D11Texture2D> dsBuffer;
	D3D11_TEXTURE2D_DESC desc_dsBuffer = {};
	desc_dsBuffer.Width = MainWindow::GetDisplayResolution().width;
	desc_dsBuffer.Height = MainWindow::GetDisplayResolution().height;
	desc_dsBuffer.MipLevels = 1u;
	desc_dsBuffer.ArraySize = 1u;
	desc_dsBuffer.Format = DXGI_FORMAT_D32_FLOAT;
	desc_dsBuffer.SampleDesc.Count = 1u;
	desc_dsBuffer.SampleDesc.Quality = 0u;
	desc_dsBuffer.Usage = D3D11_USAGE_DEFAULT;
	desc_dsBuffer.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	CHECK_DX_ERROR(pDevice->CreateTexture2D(&desc_dsBuffer, nullptr, &dsBuffer));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
	dsViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u;
	CHECK_DX_ERROR(pDevice->CreateDepthStencilView(dsBuffer.Get(), &dsViewDesc, pDepthStencil.GetAddressOf()));

	//Render target creation
	Microsoft::WRL::ComPtr<ID3D11Texture2D> resource = nullptr;
	CHECK_DX_ERROR(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &resource));
	CHECK_DX_ERROR(pDevice->CreateRenderTargetView(resource.Get(), nullptr, &pView));
	pDeviceContext->OMSetRenderTargets(1u, pView.GetAddressOf(), pDepthStencil.Get());


	//Initializing projection matrix
	SetProjection(ProjectionType::Perspective, 16.0f / 9.0f , 60.0f, 0.05f, 10000.0f);

	//Initializing view matrix.
	//If we are in editor mode we will use editor camera matrix as our view matrix.
	#ifdef EDITOR
	viewMatrix = &Editor::Camera::TransformMatrix;
	#else
	//TODO: set viewmatrix to game camera's view matrix.
	#endif

	//Adjusting clear render target view mode
	SetClearMode(ClearMode::Grey);

	//Ambient Lighting
	ambientLight = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
	ambientLightBuffer = new PS_ConstantBuffer<DirectX::XMVECTOR>(&ambientLight, 1u, 0u, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,true);

	InitRS();
	InitBS();

	ENGINE_LOG(ENGINE_INFO, "Graphics Ready!");
}

void Graphics::EndFrame()
{
	//In debug mode present is not working when window is minimized.
#ifndef NDEBUG
	if (MainWindow::isMinimized())
	{
		CHECK_DX_ERROR(pSwapChain->Present(isVSyncEnabled, 0u));
	}
#else
	pSwapChain->Present(isVSyncEnabled, 0u);
#endif
}

void Graphics::BeginFrame()
{
	pDeviceContext->ClearRenderTargetView(pView.Get(), clearColor);
	pDeviceContext->ClearDepthStencilView(pDepthStencil.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}

void Graphics::Enable_VSYNC()
{
	isVSyncEnabled = true;
}

void Graphics::Disable_VSYNC()
{
	isVSyncEnabled = false;
}

void Graphics::SetClearMode(ClearMode cMode)
{
	clearMode = cMode;

	switch (clearMode)
	{
	case Graphics::ClearMode::Black:
		clearColor[0] = 0.0f;
		clearColor[1] = 0.0f;
		clearColor[2] = 0.0f;
		clearColor[3] = 1.0f;
		break;
	case Graphics::ClearMode::Grey:
		clearColor[0] = 0.22f;
		clearColor[1] = 0.22f;
		clearColor[2] = 0.22f;
		clearColor[3] = 1.0f;
		break;
	default:
		break;
	}
}

float Graphics::GetAspectRatio()
{
	return aspectRatio;
}

float Graphics::GetVerticalFOV()
{
	if (projectionType == ProjectionType::Perspective)
		return verticalFOV;
	else
		return -1.0f;
}

float Graphics::GetHorizontalFOV()
{
	if (projectionType == ProjectionType::Perspective)
		return horizontalFOV;
	else
		return -1.0f;
}


void Graphics::SetProjection(ProjectionType type, float aspectRatio, float height,float near_z,float far_z)
{
	projectionType = type;
	switch (projectionType)
	{
	case Graphics::ProjectionType::Orthographic:
		Graphics::aspectRatio = aspectRatio;
		projectionMatrix = DirectX::XMMatrixOrthographicLH(height * aspectRatio, height , near_z, far_z);
		break;
	case Graphics::ProjectionType::Perspective:
		Graphics::aspectRatio = aspectRatio;
		verticalFOV = height;
		horizontalFOV = DirectX::XMConvertToDegrees(2.0f * std::atan(aspectRatio * std::tan(DirectX::XMConvertToRadians(height / 2.0f))));
		projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(verticalFOV),aspectRatio,near_z, far_z);
		break;
	default:
		ENGINEASSERT(false, "Invalid ProjectionType");
		break;
	}
}

void Graphics::SetAmbientColor(DirectX::XMFLOAT3 color)
{
	ambientLight = DirectX::XMVectorSet(color.x, color.y, color.z, GetAmbientIntensity());
	ambientLightBuffer->ChangeData(&ambientLight);
}

void Graphics::SetAmbientIntensity(float intensity)
{
	ambientLight = DirectX::XMVectorSetW(ambientLight, intensity);
	ambientLightBuffer->ChangeData(&ambientLight);
}

DirectX::XMFLOAT3 Graphics::GetAmbientColor()
{
	DirectX::XMFLOAT3 color;
	DirectX::XMStoreFloat3(&color, ambientLight);
	return color;
}

float Graphics::GetAmbientIntensity()
{
	return DirectX::XMVectorGetW(ambientLight);
}

const Unlit_Material& Graphics::GetWireframeMaterial()
{
	static Unlit_Material wireframeMaterial("Graphics\\wireframeMaterial");
	wireframeMaterial.SetColor( { 0.0f, 0.5f, 0.0f, 1.0f } );
	
	return wireframeMaterial;
}

void Graphics::InitRS()
{
	D3D11_RASTERIZER_DESC solid;
	ZeroMemory(&solid, sizeof(D3D11_RASTERIZER_DESC));

	solid.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	solid.CullMode = D3D11_CULL_BACK;
	solid.FrontCounterClockwise = FALSE;
	solid.DepthClipEnable = TRUE;
	
	pDevice->CreateRasterizerState(&solid, &RS_Solid);

	////////////////////////////////////////////////////

	D3D11_RASTERIZER_DESC wireframe;
	ZeroMemory(&wireframe, sizeof(D3D11_RASTERIZER_DESC));

	wireframe.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	wireframe.CullMode = D3D11_CULL_BACK;
	wireframe.FrontCounterClockwise = FALSE;
	wireframe.DepthClipEnable = TRUE;

	pDevice->CreateRasterizerState(&wireframe, &RS_Wireframe);
}

void Graphics::InitBS()
{
	D3D11_BLEND_DESC opaque;
	opaque.AlphaToCoverageEnable = FALSE;
	opaque.IndependentBlendEnable = FALSE;
	opaque.RenderTarget[0].BlendEnable = FALSE;
	opaque.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	opaque.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
	opaque.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	opaque.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	opaque.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
	opaque.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	opaque.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
	pDevice->CreateBlendState(&opaque, &BS_Opaque);

	
	D3D11_BLEND_DESC transparent;
	transparent.AlphaToCoverageEnable = FALSE;
	transparent.IndependentBlendEnable = FALSE;
	transparent.RenderTarget[0].BlendEnable = TRUE;
	transparent.RenderTarget[0].SrcBlend = D3D11_BLEND::D3D11_BLEND_SRC_ALPHA;
	transparent.RenderTarget[0].DestBlend = D3D11_BLEND::D3D11_BLEND_INV_SRC_ALPHA;
	transparent.RenderTarget[0].BlendOp = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	transparent.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND::D3D11_BLEND_ONE;
	transparent.RenderTarget[0].DestBlendAlpha = D3D11_BLEND::D3D11_BLEND_ZERO;
	transparent.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP::D3D11_BLEND_OP_ADD;
	transparent.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE::D3D11_COLOR_WRITE_ENABLE_ALL;
	pDevice->CreateBlendState(&transparent, &BS_Transparent);
}
