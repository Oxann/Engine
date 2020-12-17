#pragma once

#include "D3DBase.h"

class IndexBuffer final : public D3DBase
{
public:
	IndexBuffer(const unsigned short* data, unsigned int count)
	{
		ENGINEASSERT(count >= 1, "Index Buffer size cannot be less than 1.");
		ENGINEASSERT(data != nullptr, "Index Buffer Data cannot be null pointer.");

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = sizeof(unsigned short) * count;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.StructureByteStride = 0u;
		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = data;
		CHECK_DX_ERROR(GetDevice()->CreateBuffer(&bufferDesc, &subData, &indexBuffer));
	}

	//Slot is not used.
	void BindPipeline() const override
	{
		GetDeviceContext()->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);
	}
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
};