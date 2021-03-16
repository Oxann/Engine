#pragma once
#include "D3DBase.h"
#include "EngineAssert.h"
#include <DirectXMath.h>

template <class DataType>
class ConstantBuffer : public D3DBase
{
public:
	ConstantBuffer(const DataType* data, UINT size, UINT slot, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT cpuAccess = 0u)
		:size(size),
		slot(slot)
	{
		ENGINEASSERT(slot >= 0, "Constant buffer slot cannt be less than 0.");
		ENGINEASSERT(size >= 1, "Constant Buffer size cannot be less than 1.");
		ENGINEASSERT(data != nullptr, "Constant Buffer Data cannot be null pointer.");

		D3D11_BUFFER_DESC cbdesc;
		cbdesc.ByteWidth = sizeof(DataType) * size;
		cbdesc.Usage = usage;
		cbdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbdesc.CPUAccessFlags = cpuAccess;
		cbdesc.MiscFlags = 0u;
		cbdesc.StructureByteStride = sizeof(DataType);
		D3D11_SUBRESOURCE_DATA cbdata;
		cbdata.pSysMem = data;
		CHECK_DX_ERROR(GetDevice()->CreateBuffer(&cbdesc, &cbdata, &buffer));
	}

	void ChangeData(const DataType* data)
	{
		D3D11_MAPPED_SUBRESOURCE resource;
		GetDeviceContext()->Map(buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &resource);
		memcpy(resource.pData, data, sizeof(*data) * size);
		GetDeviceContext()->Unmap(buffer.Get(), 0u);
	}

	void BindPipeline() const = 0;
protected:
	Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
	UINT size;
	UINT slot;
};


template<class DataType>
class VS_ConstantBuffer : public ConstantBuffer<DataType>
{
public:
	VS_ConstantBuffer(const DataType* data, UINT size, UINT slot, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT cpuAccess = 0u, bool bind = false)
		:ConstantBuffer<DataType>(data,size,slot,usage,cpuAccess)
	{
		if (bind)
			BindPipeline();
	}
	void BindPipeline() const override
	{
		D3DBase::GetDeviceContext()->VSSetConstantBuffers(ConstantBuffer<DataType>::slot, 1u, ConstantBuffer<DataType>::buffer.GetAddressOf());
	}
};

template<class DataType>
class PS_ConstantBuffer : public ConstantBuffer<DataType>
{
public:
	PS_ConstantBuffer(const DataType* data, UINT size, UINT slot, D3D11_USAGE usage = D3D11_USAGE_DEFAULT, UINT cpuAccess = 0u, bool bind = false)
		:ConstantBuffer<DataType>(data, size, slot, usage, cpuAccess)
	{
		if (bind)
			BindPipeline();
	}
	void BindPipeline() const override
	{
		D3DBase::GetDeviceContext()->PSSetConstantBuffers(ConstantBuffer<DataType>::slot, 1u, ConstantBuffer<DataType>::buffer.GetAddressOf());
	}
};



