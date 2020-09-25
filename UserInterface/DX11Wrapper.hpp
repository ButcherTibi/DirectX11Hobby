#pragma once

// DirectX 11
#include <d3d11_4.h>

// ComPtr
#include <wrl\client.h>

#include "ErrorStack.hpp"


template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace dx11 {

	class Buffer {
		ID3D11Device5* dev;
		ID3D11DeviceContext4* ctx;
		D3D11_BUFFER_DESC init_desc;

	public:
		ComPtr<ID3D11Buffer> buff = nullptr;

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext4* ctx, D3D11_BUFFER_DESC& desc);
		nui::ErrStack load(void* data, size_t load_size, uint32_t sub_resource_idx = 0);
	};

	nui::ErrStack singleLoad(ID3D11DeviceContext4* ctx, ID3D11Resource* resource,
		void* data, size_t load_size, uint32_t sub_resource_idx = 0);
}
