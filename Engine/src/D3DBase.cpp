#include "D3DBase.h"
#include "Graphics.h"

Microsoft::WRL::ComPtr<ID3D11Device> D3DBase::GetDevice()
{
	return Graphics::pDevice;
}

Microsoft::WRL::ComPtr<ID3D11DeviceContext> D3DBase::GetDeviceContext()
{
	return Graphics::pDeviceContext;
}
