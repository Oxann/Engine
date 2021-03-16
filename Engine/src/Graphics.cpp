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
#include "Renderer.h"


void Graphics::Init(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = MainWindow::GetDisplayResolution().width;
	swapChainDesc.BufferDesc.Height = MainWindow::GetDisplayResolution().height;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;
	swapChainDesc.OutputWindow = hWnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDesc.Flags = 0;


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
		&swapChainDesc,
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
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	pDeviceContext->RSSetViewports(1, &vp);

	//Depth and stencil buffer
	Microsoft::WRL::ComPtr<ID3D11Texture2D> dsBuffer;
	D3D11_TEXTURE2D_DESC desc_dsBuffer = {};
	desc_dsBuffer.Width = MainWindow::GetDisplayResolution().width;
	desc_dsBuffer.Height = MainWindow::GetDisplayResolution().height;
	desc_dsBuffer.MipLevels = 1u;
	desc_dsBuffer.ArraySize = 1u;
	desc_dsBuffer.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	desc_dsBuffer.SampleDesc.Count = 1u;
	desc_dsBuffer.SampleDesc.Quality = 0u;
	desc_dsBuffer.Usage = D3D11_USAGE_DEFAULT;
	desc_dsBuffer.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	CHECK_DX_ERROR(pDevice->CreateTexture2D(&desc_dsBuffer, nullptr, &dsBuffer));

	D3D11_DEPTH_STENCIL_VIEW_DESC dsViewDesc = {};
	dsViewDesc.Format = DXGI_FORMAT::DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsViewDesc.Texture2D.MipSlice = 0u;
	CHECK_DX_ERROR(pDevice->CreateDepthStencilView(dsBuffer.Get(), &dsViewDesc, pDepthStencil.GetAddressOf()));

	//Render target creation
	Microsoft::WRL::ComPtr<ID3D11Texture2D> resource = nullptr;
	CHECK_DX_ERROR(pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &resource));
	CHECK_DX_ERROR(pDevice->CreateRenderTargetView(resource.Get(), nullptr, &pView));
	pDeviceContext->OMSetRenderTargets(1u, pView.GetAddressOf(), pDepthStencil.Get());


	//Initializing projection matrix
	SetProjection(ProjectionType::Perspective, (float)MainWindow::GetDisplayResolution().width / (float)MainWindow::GetDisplayResolution().height, 60.0f, 0.05f, 10000.0f);

	//Ambient Lighting
	ambientLight = DirectX::XMVectorSet(1.0f, 1.0f, 1.0f, 0.5f);
	ambientLightBuffer = new PS_ConstantBuffer<DirectX::XMVECTOR>(&ambientLight, 1u, 0u, D3D11_USAGE::D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE,true);
	
	ENGINE_LOG(ENGINE_INFO, "Graphics Ready!");
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