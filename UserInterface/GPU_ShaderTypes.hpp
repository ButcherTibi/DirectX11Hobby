#pragma once

#include "pch.h"

#include <DirectXMath.h>


namespace nui {

	struct GPU_Vertex {
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
}
