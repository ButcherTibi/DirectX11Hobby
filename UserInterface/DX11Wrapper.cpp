
#include "pch.h"

// Header
#include "DX11Wrapper.hpp"


using namespace dx11;

void Buffer::create(ID3D11Device5* device, ID3D11DeviceContext3* context, D3D11_BUFFER_DESC& desc)
{
	this->dev = device;
	this->ctx = context;
	this->init_desc = desc;
}

ErrStack Buffer::load(void* data, size_t load_size, uint32_t sub_resource_idx)
{
	ErrStack err_stack;
	HRESULT hr = S_OK;

	// Create New Buffer of load size
	auto create_buff = [&]() -> ErrStack {

		init_desc.ByteWidth = load_size;
		checkHResult(dev->CreateBuffer(&init_desc, NULL, buff.GetAddressOf()),
			"failed to create vertex buffer");

		return ErrStack();
	};

	if (buff == nullptr) {
		checkErrStack1(create_buff());
	}
	else {
		D3D11_BUFFER_DESC desc;
		buff->GetDesc(&desc);

		if(load_size > desc.ByteWidth) {

			buff->Release();
			checkErrStack1(create_buff());
		}
	}

	D3D11_MAPPED_SUBRESOURCE mapped;
	checkHResult(ctx->Map(buff.Get(), sub_resource_idx, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
		"failed to map resource");

	std::memcpy(mapped.pData, data, load_size);

	ctx->Unmap(buff.Get(), sub_resource_idx);

	return err_stack;
}

ErrStack dx11::singleLoad(ID3D11DeviceContext3* ctx, ID3D11Resource* resource,
	void* data, size_t load_size, uint32_t sub_resource_idx)
{
	HRESULT hr = S_OK;
	ErrStack err_stack;

	D3D11_MAPPED_SUBRESOURCE mapped;
	checkHResult(ctx->Map(resource, sub_resource_idx, D3D11_MAP_WRITE_DISCARD, 0, &mapped),
		"failed to map resource");

	std::memcpy(mapped.pData, data, load_size);

	ctx->Unmap(resource, sub_resource_idx);

	return err_stack;
}

ErrStack dx11::resizeTexture2D(ID3D11Device5* dev, uint32_t new_width, uint32_t new_height,
	ComPtr<ID3D11Texture2D>& tex)
{
	HRESULT hr = S_OK;

	D3D11_TEXTURE2D_DESC desc;
	tex->GetDesc(&desc);

	tex->Release();

	desc.Width = new_width;
	desc.Height = new_height;
	checkHResult(dev->CreateTexture2D(&desc, NULL, tex.GetAddressOf()),
		"failed to resize texture 2D");

	return ErrStack();
}
