#pragma once

// Standard
#include <array>

// GLM
#include <glm\vec3.hpp>

// DirectX 11
#include <DirectXMath.h>
#include <d3d11_4.h>


/*FORCEINLINE*/ DirectX::XMFLOAT3 dxConvert(glm::vec3& value);


struct GPU_MeshVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 2> getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
		elems[0].SemanticName = "POSITION";
		elems[0].SemanticIndex = 0;
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elems[0].InputSlot = 0;
		elems[0].AlignedByteOffset = 0;
		elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elems[0].InstanceDataStepRate = 0;

		elems[1].SemanticName = "NORMAL";
		elems[1].SemanticIndex = 0;
		elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elems[1].InputSlot = 0;
		elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elems[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elems[1].InstanceDataStepRate = 0;

		return elems;
	}
};


struct GPU_MeshInstance {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 rot;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 2> getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
		elems[0].SemanticName = "INSTANCE_POSITION";
		elems[0].SemanticIndex = 0;
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elems[0].InputSlot = 0;
		elems[0].AlignedByteOffset = 0;
		elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elems[0].InstanceDataStepRate = 0;

		elems[1].SemanticName = "INSTANCE_ROTATION";
		elems[1].SemanticIndex = 0;
		elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elems[1].InputSlot = 0;
		elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elems[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		elems[1].InstanceDataStepRate = 0;

		return elems;
	}
};


struct GPU_MeshUniform {
	DirectX::XMFLOAT4 camera_quat;
	DirectX::XMMATRIX perspective_matrix;
};
