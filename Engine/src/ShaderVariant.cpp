#include "ShaderVariant.h"
#include "Graphics.h"
#include "EngineAssert.h"
#include <d3dcompiler.h>
#include "Shader.h"

using namespace Microsoft::WRL;

VertexShaderVariant::VertexShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	CHECK_DX_ERROR(Graphics::pDevice->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &vertexShader));
	InitInputLayoutAndBuffers(blob);
}

void VertexShaderVariant::Bind() const
{
	Graphics::pDeviceContext->VSSetShader(vertexShader.Get(), nullptr, 0u);
	Graphics::pDeviceContext->IASetInputLayout(inputLayout.Get());
}

bool VertexShaderVariant::HasShadows() const
{
	return hasShadows;
}

const std::vector<VertexBuffer::ElementType>& VertexShaderVariant::GetVertexElements() const
{
	return vertexElements;
}

void VertexShaderVariant::InitInputLayoutAndBuffers(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	//Reflection
	ComPtr<ID3D11ShaderReflection> vertexShaderReflection;
	CHECK_DX_ERROR(D3DReflect(blob->GetBufferPointer(), blob->GetBufferSize(), IID_ID3D11ShaderReflection, &vertexShaderReflection));

	//Desc
	D3D11_SHADER_DESC vertexShaderDesc;
	vertexShaderReflection->GetDesc(&vertexShaderDesc);

	//Input layout from desc
	std::vector<D3D11_INPUT_ELEMENT_DESC> ilo;
	for (unsigned int i = 0; i < vertexShaderDesc.InputParameters; i++)
	{
		D3D11_SIGNATURE_PARAMETER_DESC parameterDesc;
		vertexShaderReflection->GetInputParameterDesc(i, &parameterDesc);

		D3D11_INPUT_ELEMENT_DESC temp_ilo;
		temp_ilo.SemanticName = parameterDesc.SemanticName;
		temp_ilo.SemanticIndex = parameterDesc.SemanticIndex;
		temp_ilo.InstanceDataStepRate = 0;
		temp_ilo.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		temp_ilo.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		std::string semanticName = temp_ilo.SemanticName;
		if (semanticName == "POSITION")
		{
			temp_ilo.InputSlot = 0u;
			switch (parameterDesc.Mask)
			{
			case 1:
				temp_ilo.Format = DXGI_FORMAT_R32_FLOAT;
				break;
			case 3:
				temp_ilo.Format = DXGI_FORMAT_R32G32_FLOAT;
				vertexElements.emplace_back(VertexBuffer::ElementType::Position2D);
				break;
			case 7:
				temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
				vertexElements.emplace_back(VertexBuffer::ElementType::Position3D);
				break;
			case 15:
				temp_ilo.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				break;
			}
		}
		else if (semanticName == "TEXCOORD")
		{
			temp_ilo.InputSlot = 1u;
			temp_ilo.Format = DXGI_FORMAT_R32G32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::TexCoord);
		}
		else if (semanticName == "NORMAL")
		{
			temp_ilo.InputSlot = 2u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Normal);
		}
		else if (semanticName == "TANGENT")
		{
			temp_ilo.InputSlot = 3u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Tangent);
		}
		else if (semanticName == "BITANGENT")
		{
			temp_ilo.InputSlot = 4u;
			temp_ilo.Format = DXGI_FORMAT_R32G32B32_FLOAT;
			vertexElements.emplace_back(VertexBuffer::ElementType::Bitangent);
		}
		ilo.push_back(temp_ilo);
	}
	CHECK_DX_ERROR(Graphics::pDevice->CreateInputLayout(ilo.data(), (UINT)ilo.size(), blob->GetBufferPointer(), blob->GetBufferSize(), &inputLayout));

	//Reflecting transform buffer
	ID3D11ShaderReflectionConstantBuffer* perObjectBuffer = vertexShaderReflection->GetConstantBufferByName(Shader::VertexShaderPerObjectBuffer::bufferName.c_str());

	auto modelMatrix = perObjectBuffer->GetVariableByName(Shader::VertexShaderPerObjectBuffer::modelName.c_str());
	D3D11_SHADER_VARIABLE_DESC modelMatrixDesc;
	modelMatrix->GetDesc(&modelMatrixDesc);
	perObjectBufferUsageFlags |= modelMatrixDesc.uFlags & D3D_SVF_USED ? PerObjectBufferUsageFlags::ModelMatrix : PerObjectBufferUsageFlags::None;

	auto modelViewMatrix = perObjectBuffer->GetVariableByName(Shader::VertexShaderPerObjectBuffer::modelViewName.c_str());
	D3D11_SHADER_VARIABLE_DESC modelViewMatrixDesc;
	modelViewMatrix->GetDesc(&modelViewMatrixDesc);
	perObjectBufferUsageFlags |= modelViewMatrixDesc.uFlags & D3D_SVF_USED ? PerObjectBufferUsageFlags::ModelViewMatrix : PerObjectBufferUsageFlags::None;

	auto modelViewProjectionMatrix = perObjectBuffer->GetVariableByName(Shader::VertexShaderPerObjectBuffer::modelViewProjectionName.c_str());
	D3D11_SHADER_VARIABLE_DESC modelViewProjectionMatrixDesc;
	modelViewProjectionMatrix->GetDesc(&modelViewProjectionMatrixDesc);
	perObjectBufferUsageFlags |= modelViewProjectionMatrixDesc.uFlags & D3D_SVF_USED ? PerObjectBufferUsageFlags::ModelViewProjectionMatrix : PerObjectBufferUsageFlags::None;

	auto normalMatrix = perObjectBuffer->GetVariableByName(Shader::VertexShaderPerObjectBuffer::normalMatrixName.c_str());
	D3D11_SHADER_VARIABLE_DESC normalMatrixDesc;
	normalMatrix->GetDesc(&normalMatrixDesc);
	perObjectBufferUsageFlags |= modelMatrixDesc.uFlags & D3D_SVF_USED ? PerObjectBufferUsageFlags::NormalMatrix : PerObjectBufferUsageFlags::None;

	//Shadow buffer
	ID3D11ShaderReflectionConstantBuffer* shadowBuffer = vertexShaderReflection->GetConstantBufferByName(Shader::VertexShaderShadowBuffer::bufferName.c_str());
	
	D3D11_SHADER_BUFFER_DESC shadowBufferDesc;

	//Check if the current variant has shadows
	hasShadows = shadowBuffer->GetDesc(&shadowBufferDesc) != E_FAIL;
}

PixelShaderVariant::PixelShaderVariant(Microsoft::WRL::ComPtr<ID3DBlob> blob)
{
	CHECK_DX_ERROR(Graphics::pDevice->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &pixelShader));
	InitTextureLayout(blob);
}

void PixelShaderVariant::Bind() const
{
	Graphics::pDeviceContext->PSSetShader(pixelShader.Get(), nullptr, 0u);
}

const std::vector<PixelShaderVariant::ResourceDefinition>& PixelShaderVariant::GetTexture2Ds() const
{
	return texture2Ds;
}

void PixelShaderVariant::InitTextureLayout(Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob)
{
	//Reflection
	ComPtr<ID3D11ShaderReflection> pixelShaderReflection;
	CHECK_DX_ERROR(D3DReflect(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), IID_ID3D11ShaderReflection, &pixelShaderReflection));

	D3D11_SHADER_DESC desc;
	pixelShaderReflection->GetDesc(&desc);

	for (int i = 0; i < desc.BoundResources; i++)
	{
		D3D11_SHADER_INPUT_BIND_DESC resourceDesc;
		pixelShaderReflection->GetResourceBindingDesc(i, &resourceDesc);
		switch (resourceDesc.Type)
		{
		case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:
			switch (resourceDesc.Dimension)
			{
			case D3D_SRV_DIMENSION::D3D_SRV_DIMENSION_TEXTURE2D:
				texture2Ds.emplace_back(resourceDesc.Name, resourceDesc.BindPoint);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}
