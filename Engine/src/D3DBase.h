#pragma once
#include <wrl.h>
#include <d3d11.h>

class D3DBase
{
public:
	D3DBase() = default;
	virtual ~D3DBase() = default;
	virtual void BindPipeline() const = 0;
protected:
	static Microsoft::WRL::ComPtr<ID3D11Device> GetDevice();
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetDeviceContext();
};