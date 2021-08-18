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
	DirectX::XMINT2 toXM(int32_t x, int32_t y);
	DirectX::XMFLOAT2 toXM(glm::vec2& value);
	DirectX::XMFLOAT4 toXM(glm::vec4& value);

	struct GPU_CharacterVertex {
		DirectX::XMINT2 pos;
		DirectX::XMFLOAT2 uv;
		uint32_t instance_id;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 3> nodes;
			nodes[0].SemanticName = "POSITION";
			nodes[0].Format = DXGI_FORMAT_R32G32_SINT;

			nodes[1].SemanticName = "TEXCOORD";
			nodes[1].Format = DXGI_FORMAT_R32G32_FLOAT;

			nodes[2].SemanticName = "INSTANCE_ID";
			nodes[2].Format = DXGI_FORMAT_R32_UINT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : nodes) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elem.InstanceDataStepRate = 0;
			}

			return nodes;
		}
	};


	struct GPU_TextInstance {
		DirectX::XMFLOAT4 color;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 1> nodes;
			nodes[0].SemanticName = "COLOR";
			nodes[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : nodes) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
				elem.InstanceDataStepRate = 1;
			}

			return nodes;
		}
	};


	struct GPU_SimpleVertex {
		DirectX::XMINT2 pos;
		uint32_t instance_id;

		static auto getInputLayout(uint32_t input_slot = 0)
		{
			std::array<D3D11_INPUT_ELEMENT_DESC, 2> nodes;
			nodes[0].SemanticName = "POSITION";
			nodes[0].Format = DXGI_FORMAT_R32G32_SINT;

			nodes[1].SemanticName = "INSTANCE_ID";
			nodes[1].Format = DXGI_FORMAT_R32_UINT;

			for (D3D11_INPUT_ELEMENT_DESC& elem : nodes) {
				elem.SemanticIndex = 0;
				elem.InputSlot = input_slot;
				elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
				elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
				elem.InstanceDataStepRate = 0;
			}

			return nodes;
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


	struct GPU_RectInstance {
		DirectX::XMFLOAT4 color;
	};

	namespace GPU_ConstantsFields {
		enum {
			SCREEN_WIDTH,
			SCREEN_HEIGHT
		};
	}

	/* WARNING: All structs below are HLSL binary compatible with shader constant buffer */
	//#pragma pack(16)
	//
	//#pragma pack()
}
