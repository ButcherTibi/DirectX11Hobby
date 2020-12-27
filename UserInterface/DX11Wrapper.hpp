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
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext4* im_ctx4;
		D3D11_BUFFER_DESC init_desc;

		ComPtr<ID3D11Buffer> buff = nullptr;

		uint32_t sub_resource_idx;

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext4* ctx, D3D11_BUFFER_DESC& desc);
		ErrStack load(void* data, size_t load_size, uint32_t sub_resource_idx = 0);

		ErrStack beginLoad(size_t total_load_size, uint32_t sub_resource_idx,
			void*& mapped_buffer);

		void endLoad();

		size_t getMemorySizeMegaBytes();
	};

	ErrStack singleLoad(ID3D11DeviceContext3* ctx, ID3D11Resource* resource,
		void* data, size_t load_size, uint32_t sub_resource_idx = 0);

	ErrStack resizeTexture2D(ID3D11Device5* dev, uint32_t new_width, uint32_t new_height,
		ComPtr<ID3D11Texture2D>& tex);
}
