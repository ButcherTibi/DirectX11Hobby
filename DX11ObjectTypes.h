#pragma once

// ComPtr
#include <d3d11.h>
#include <wrl\client.h>

// DX11
#include <DirectXMath.h>

// Mine
#include "ErrorStuff.h"


namespace nui_int {

	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	typedef DirectX::XMFLOAT2 xm_float2;
	typedef DirectX::XMFLOAT4 xm_float4;


	struct BufferCreateInfo {
		D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
		uint32_t bind_flags = 0;
		uint32_t cpu_access_flags = 0;
		uint32_t misc_flags = 0;
	};

	template<typename T>
	class VectorBuffer {
	public:
		ID3D11Device* dev;
		ID3D11DeviceContext* ctx;

		BufferCreateInfo info;

		ComPtr<ID3D11Buffer> buff = nullptr;
		size_t count = 0;
		uint32_t stride;

		void create(ID3D11Device* dev, ID3D11DeviceContext* ctx, BufferCreateInfo& info);

		ErrStack fill(const std::vector<T>& values, bool& recreated);
	};

	template<typename T>
	void VectorBuffer<T>::create(ID3D11Device* dev, ID3D11DeviceContext* ctx, BufferCreateInfo& info)
	{
		this->dev = dev;
		this->ctx = ctx;

		this->info = info;
		this->count = 0;
		this->stride = sizeof(T);
	}

	template<typename T>
	ErrStack VectorBuffer<T>::fill(const std::vector<T>& values, bool& recreated)
	{
		ErrStack err_stack{};
		HRESULT hr{};

		if (values.size() > count) {

			if (buff != nullptr) {
				buff->Release();
			}

			size_t size = 0;
			if (info.bind_flags == D3D11_BIND_CONSTANT_BUFFER) {
				size = values.size() * sizeof(T);
				size += size % 16;  // round to 16 bytes
			}
			else {
				size = values.size() * sizeof(T);
			}

			D3D11_BUFFER_DESC buff_desc = {};
			buff_desc.ByteWidth = size;
			buff_desc.Usage = info.usage;
			buff_desc.BindFlags = info.bind_flags;
			buff_desc.CPUAccessFlags = info.cpu_access_flags;
			buff_desc.MiscFlags = info.misc_flags;
			buff_desc.StructureByteStride = sizeof(T);

			D3D11_SUBRESOURCE_DATA subresource_data = {};
			subresource_data.pSysMem = values.data();

			checkHResult(dev->CreateBuffer(&buff_desc, &subresource_data, buff.GetAddressOf()),
				"failed to create vector buffer");

			recreated = true;
		}
		else {
			D3D11_MAPPED_SUBRESOURCE mapped_res = {};
			checkHResult(ctx->Map(buff.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_res),
				"failed to map vector buffer");

			std::memcpy(mapped_res.pData, values.data(), values.size() * sizeof(T));

			ctx->Unmap(buff.Get(), 0);

			recreated = false;
		}

		return err_stack;
	}


	template<typename T>
	class FixedBuffer {
	public:
		ID3D11Device* dev;
		ID3D11DeviceContext* ctx;

		BufferCreateInfo info;

		ComPtr<ID3D11Buffer> buff = nullptr;

		void create(ID3D11Device* dev, ID3D11DeviceContext* ctx, BufferCreateInfo& info);

		ErrStack fill(T& new_value, bool& recreated);
	};

	template<typename T>
	void FixedBuffer<T>::create(ID3D11Device* dev, ID3D11DeviceContext* ctx, BufferCreateInfo& info)
	{
		this->dev = dev;
		this->ctx = ctx;
		this->info = info;
	}

	template<typename T>
	ErrStack FixedBuffer<T>::fill(T& new_value, bool& recreated)
	{
		ErrStack err_stack{};
		HRESULT hr{};

		if (buff == nullptr) {

			size_t size = 0;
			if (info.bind_flags == D3D11_BIND_CONSTANT_BUFFER) {
				size = sizeof(T);
				size += size % 16;  // round to 16 bytes
			}
			else {
				size = sizeof(T);
			}

			D3D11_BUFFER_DESC buff_desc = {};
			buff_desc.ByteWidth = size;
			buff_desc.Usage = info.usage;
			buff_desc.BindFlags = info.bind_flags;
			buff_desc.CPUAccessFlags = info.cpu_access_flags;
			buff_desc.MiscFlags = info.misc_flags;
			buff_desc.StructureByteStride = size;

			D3D11_SUBRESOURCE_DATA subresource_data = {};
			subresource_data.pSysMem = &new_value;

			checkHResult(dev->CreateBuffer(&buff_desc, &subresource_data, buff.GetAddressOf()),
				"failed to create fixed buffer");

			recreated = true;
		}
		else {
			D3D11_MAPPED_SUBRESOURCE mapped_res = {};
			checkHResult(ctx->Map(buff.Get(), 0, D3D11_MAP::D3D11_MAP_WRITE_DISCARD, 0, &mapped_res),
				"failed to map fixed buffer");

			std::memcpy(mapped_res.pData, &new_value, sizeof(T));

			ctx->Unmap(buff.Get(), 0);

			recreated = false;
		}

		return err_stack;
	}


	struct GPU_Vertex {
		xm_float2 pos;
		uint32_t idx;
	};


	struct GPU_RectProps {
		xm_float4 color;
	};

	struct GPU_CircleProps {
		xm_float4 color;
		xm_float4 center_radius;
	};

	struct GPU_CommonStuff {
		uint32_t width;
		DirectX::XMFLOAT3 _pad1;

		uint32_t height;
		DirectX::XMFLOAT3 _pad2;
	};


	class VertexShader {
	public:
		ComPtr<ID3D11VertexShader> shader;
		std::vector<uint8_t> code;

	public:
		ErrStack create(ID3D11Device* device, std::string path);
	};


	class PixelShader {
	public:
		ComPtr<ID3D11PixelShader> shader;
		std::vector<uint8_t> code;

	public:
		ErrStack create(ID3D11Device* device, std::string path);
	};
}
