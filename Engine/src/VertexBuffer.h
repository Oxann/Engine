#pragma once
#include "D3DBase.h"
#include "EngineAssert.h"
#include <DirectXMath.h>


class VertexBuffer final : public D3DBase
{
public:
	enum class ElementType
	{
		Position2D,
		Position3D,
		TexCoord,
		Normal,
		Tangent,
		Bitangent,
		BoneWeights,
		BoneIDs
	};
public:
	VertexBuffer(const void* data, unsigned int count,ElementType type)
	{	
		ENGINEASSERT(count >= 1, "Vertex Buffer size cannot be less than 1.");
		ENGINEASSERT(data != nullptr, "Vertex Buffer Data cannot be null pointer.");

		switch (type)
		{
		case VertexBuffer::ElementType::Position2D:
			slot = 0u;
			stride = sizeof(float) * 2u;
			break;
		case VertexBuffer::ElementType::Position3D:
			slot = 0u;
			stride = sizeof(float) * 3u;
			break;
		case VertexBuffer::ElementType::TexCoord:
			slot = 1u;
			stride = sizeof(float) * 2u;
			break;
		case VertexBuffer::ElementType::Normal:
			slot = 2u;
			stride = sizeof(float) * 3u;
			break;
		case VertexBuffer::ElementType::Tangent:
			slot = 3u;
			stride = sizeof(float) * 3u;
			break;
		case VertexBuffer::ElementType::Bitangent:
			slot = 4u;
			stride = sizeof(float) * 3u;
			break;
		case VertexBuffer::ElementType::BoneIDs:
			slot = 5u;
			stride = sizeof(float) * 4u;
			break;
		case VertexBuffer::ElementType::BoneWeights:
			slot = 6u;
			stride = sizeof(unsigned int) * 4u;
			break;
		default:
			ENGINEASSERT(false, "Vertex buffer type is not correct.");
			break;
		}

		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.ByteWidth = stride * count;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0u;
		bufferDesc.MiscFlags = 0u;
		bufferDesc.StructureByteStride = stride;
		D3D11_SUBRESOURCE_DATA subData = {};
		subData.pSysMem = data;
		CHECK_DX_ERROR(GetDevice()->CreateBuffer(&bufferDesc, &subData, &vertexBuffer));
	}

	void BindPipeline() const override
	{
		UINT offset = 0;
		GetDeviceContext()->IASetVertexBuffers(slot, 1u, vertexBuffer.GetAddressOf(), &stride, &offset);
	}

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
	UINT stride = 0u;
	UINT slot = 0u;
};
