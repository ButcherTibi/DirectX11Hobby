#pragma once

// Standard
#include <array>

// DirectX 11
#include <DirectXMath.h>
#include <d3d11_4.h>

// GLM
#include <glm\vec2.hpp>

namespace nui {

	DirectX::XMUINT2 toXM(glm::uvec2& value);
	DirectX::XMINT2 toXM(glm::ivec2& value);
	DirectX::XMINT2 toXM(uint32_t x, int32_t y);
	DirectX::XMFLOAT2 toXM(glm::vec2& value);
	DirectX::XMFLOAT4 toXM(glm::vec4& value);

	struct GPU_CharacterVertex {
		DirectX::XMINT2 pos;
		DirectX::XMFLOAT2 uv;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
			elems[0].SemanticName = "POSITION";
			elems[0].Format = DXGI_FORMAT_R32G32_SINT;

			elems[1].SemanticName = "TEXCOORD";
			elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elem.InstanceDataStepRate = 0;
			}

			return elems;
		}
	};


	struct GPU_TextInstance {
		DirectX::XMFLOAT4 color;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 1> elems;
			elems[0].SemanticName = "COLOR";
			elems[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				elem.InstanceDataStepRate = 1;
			}

			return elems;
		}
	};


	struct GPU_RectVertex {
		DirectX::XMINT2 pos;
		DirectX::XMFLOAT2 uv;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
			elems[0].SemanticName = "POSITION";
			elems[0].Format = DXGI_FORMAT_R32G32_SINT;

			elems[1].SemanticName = "TEXCOORD";
			elems[1].Format = DXGI_FORMAT_R32G32_FLOAT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elem.InstanceDataStepRate = 0;
			}

			return elems;
		}
	};


	namespace GPU_RectDrawcallFields {
		enum {
			POSITION_SIZE,
			COLORS,
			COLOR_LENGHTS,
			GRADIENT_ANGLE
		};
	}

	/*struct GPU_RectInstance {
		DirectX::XMFLOAT4 color_0;
		DirectX::XMFLOAT4 color_1;
		DirectX::XMFLOAT4 color_2;

		float color_0_length;
		float color_1_length;
		float color_2_length;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 6> elems;
			elems[0].SemanticName = "color_0_";
			elems[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			elems[1].SemanticName = "color_1_";
			elems[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			elems[2].SemanticName = "color_2_";
			elems[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			elems[3].SemanticName = "COLOR_0_LENGTH";
			elems[3].Format = DXGI_FORMAT_R32_FLOAT;

			elems[4].SemanticName = "COLOR_1_LENGTH";
			elems[4].Format = DXGI_FORMAT_R32_FLOAT;

			elems[5].SemanticName = "COLOR_2_LENGTH";
			elems[5].Format = DXGI_FORMAT_R32_FLOAT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				elem.InstanceDataStepRate = 1;
			}

			return elems;
		}
	};*/


/* WARNING: All structs below are HLSL binary compatible with shader constant buffer */
#pragma pack(16)

	struct GPU_Constants {
		DirectX::XMINT2 screen_size;
		uint32_t _pad0[2];
		//--------------------------------
	};
}

#pragma pack()
