#pragma once

#include "pch.h"

#include <DirectXMath.h>


namespace nui {

	struct GPU_CharacterVertex {
		glm::vec2 pos;
		glm::vec2 uv;

		static std::array<D3D11_INPUT_ELEMENT_DESC, 2> getInputLayout()
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
			elems[0].SemanticName = "POSITION";
			elems[0].SemanticIndex = 0;
			elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[0].InputSlot = 0;
			elems[0].AlignedByteOffset = 0;
			elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elems[0].InstanceDataStepRate = 0;

			elems[1].SemanticName = "TEXCOORD";
			elems[1].SemanticIndex = 0;
			elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[1].InputSlot = 0;
			elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elems[1].InstanceDataStepRate = 0;

			return elems;
		}
	};


	struct GPU_CharacterInstance {
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 pos;
		float rasterized_size;
		float size;
		uint32_t parent_clip_mask;

		static std::array<D3D11_INPUT_ELEMENT_DESC, 5> getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 5> elems;
			elems[0].SemanticName = "COLOR";
			elems[0].SemanticIndex = 0;
			elems[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			elems[0].InputSlot = input_slot;
			elems[0].AlignedByteOffset = 0;
			elems[0].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[0].InstanceDataStepRate = 1;

			elems[1].SemanticName = "INSTANCE_POSITION";
			elems[1].SemanticIndex = 0;
			elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[1].InputSlot = input_slot;
			elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[1].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[1].InstanceDataStepRate = 1;

			elems[2].SemanticName = "RASTERIZED_SIZE";
			elems[2].SemanticIndex = 0;
			elems[2].Format = DXGI_FORMAT_R32_FLOAT;
			elems[2].InputSlot = input_slot;
			elems[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[2].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[2].InstanceDataStepRate = 1;

			elems[3].SemanticName = "SIZE";
			elems[3].SemanticIndex = 0;
			elems[3].Format = DXGI_FORMAT_R32_FLOAT;
			elems[3].InputSlot = input_slot;
			elems[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[3].InstanceDataStepRate = 1;

			elems[4].SemanticName = "PARENT_CLIP_ID";
			elems[4].SemanticIndex = 0;
			elems[4].Format = DXGI_FORMAT_R32_UINT;
			elems[4].InputSlot = input_slot;
			elems[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[4].InstanceDataStepRate = 1;

			return elems;
		}
	};


	struct GPU_WrapVertex {
		DirectX::XMFLOAT2 pos;

		static std::array<D3D11_INPUT_ELEMENT_DESC, 1> getInputLayout()
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 1> elems;
			elems[0].SemanticName = "POSITION";
			elems[0].SemanticIndex = 0;
			elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[0].InputSlot = 0;
			elems[0].AlignedByteOffset = 0;
			elems[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elems[0].InstanceDataStepRate = 0;

			return elems;
		}
	};


	struct GPU_WrapInstance {
		DirectX::XMFLOAT2 pos;
		DirectX::XMFLOAT2 size;
		DirectX::XMFLOAT4 color;
		uint32_t parent_clip_id;
		uint32_t child_clip_id;

		static std::array<D3D11_INPUT_ELEMENT_DESC, 5> getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 5> elems;
			elems[0].SemanticName = "INSTANCE_POSITION";
			elems[0].SemanticIndex = 0;
			elems[0].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[0].InputSlot = input_slot;
			elems[0].AlignedByteOffset = 0;
			elems[0].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[0].InstanceDataStepRate = 1;

			elems[1].SemanticName = "SIZE";
			elems[1].SemanticIndex = 0;
			elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;
			elems[1].InputSlot = input_slot;
			elems[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[1].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[1].InstanceDataStepRate = 1;

			elems[2].SemanticName = "COLOR";
			elems[2].SemanticIndex = 0;
			elems[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			elems[2].InputSlot = input_slot;
			elems[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[2].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[2].InstanceDataStepRate = 1;

			elems[3].SemanticName = "PARENT_CLIP_ID";
			elems[3].SemanticIndex = 0;
			elems[3].Format = DXGI_FORMAT_R32_UINT;
			elems[3].InputSlot = input_slot;
			elems[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[3].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[3].InstanceDataStepRate = 1;

			elems[4].SemanticName = "CHILD_CLIP_ID";
			elems[4].SemanticIndex = 0;
			elems[4].Format = DXGI_FORMAT_R32_UINT;
			elems[4].InputSlot = input_slot;
			elems[4].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elems[4].InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elems[4].InstanceDataStepRate = 1;

			return elems;
		}
	};

	struct GPU_CommonsUniform {
		DirectX::XMFLOAT4 screen_size;
	};
}
