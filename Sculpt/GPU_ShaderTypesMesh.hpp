#pragma once

// Standard
#include <array>

// GLM
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

// DirectX 11
#include <DirectXMath.h>
#include <d3d11_4.h>


DirectX::XMFLOAT3 dxConvert(glm::vec3& value);
DirectX::XMFLOAT4 dxConvert(glm::quat& value);
DirectX::XMFLOAT4X4 dxConvert(glm::mat4& value);


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
	DirectX::XMFLOAT4 rot;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 2> getInputLayout(uint32_t input_slot = 1)
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
		elems[0].SemanticName = "INSTANCE_POSITION";
		elems[0].SemanticIndex = 0;
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elems[0].InputSlot = input_slot;
		elems[0].AlignedByteOffset = 0;
		elems[0].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elems[0].InstanceDataStepRate = 1;

		elems[1].SemanticName = "INSTANCE_ROTATION";
		elems[1].SemanticIndex = 0;
		elems[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elems[1].InputSlot = input_slot;
		elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		elems[1].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
		elems[1].InstanceDataStepRate = 1;

		return elems;
	}
};


struct GPU_MeshUniform {
	DirectX::XMFLOAT3 camera_pos;
	float pad_0;
	DirectX::XMFLOAT4 camera_quat;
	DirectX::XMFLOAT3 camera_forward;
	float pad_1;
	DirectX::XMFLOAT4X4 perspective_matrix;
};
