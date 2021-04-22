#pragma once

// DirectX 11
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// ComPtr
#include <wrl\client.h>

#include "ErrorStack.hpp"


template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace dx11 {

	class Buffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ComPtr<ID3D11Buffer> buff;

	public:
		void _ensureSize(size_t size);

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_BUFFER_DESC& desc);

		void load(void* data, size_t load_size);

		void beginLoad(size_t total_load_size, void*& mapped_mem);
		void endLoad();

		void mapForWrite(size_t total_load_size, void*& mapped_mem);
		void unmap();

		ID3D11Buffer* get();

		size_t getMemorySizeMegaBytes();
	};

	
	template<typename GPU_T>
	class ArrayBuffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ID3D11Buffer* buff = nullptr;

		D3D11_MAPPED_SUBRESOURCE mapped_mem;

	public:
		void create(ID3D11Device5* device, ID3D11DeviceContext3* context, D3D11_BUFFER_DESC& desc)
		{
			this->dev = device;
			this->ctx3 = context;
			this->init_desc = desc;
		}

		// creates a new buffer, copying the data to the new buffer
		// size is the number of elements
		void resize(size_t new_size)
		{
			size_t new_size_bytes = new_size * sizeof(GPU_T);

			// fresh buffer
			if (buff == nullptr) {

				init_desc.ByteWidth = new_size_bytes;
				throwDX11(dev->CreateBuffer(&init_desc, NULL, &buff));
			}
			else if (init_desc.ByteWidth < new_size_bytes) {

				ID3D11Buffer* new_buff;
				{
					D3D11_BUFFER_DESC desc = init_desc;
					desc.ByteWidth = new_size_bytes;

					throwDX11(dev->CreateBuffer(&init_desc, NULL, &new_buff));
				}

				D3D11_BOX src_box = {};
				src_box.left = 0;
				src_box.right = init_desc.ByteWidth;
				src_box.top = 0;
				src_box.bottom = 1;
				src_box.front = 0;
				src_box.back = 1;

				ctx3->CopySubresourceRegion(new_buff, 0,
					0, 0, 0,
					buff, 0,
					&src_box);

				buff->Release();

				buff = new_buff;

				init_desc.ByteWidth = new_size_bytes;
			}
		}

		// resizeDiscard

		/*void mapForWrite()
		{
			throwDX11(ctx3->Map(buff, 0, D3D11_MAP_WRITE, 0, &mapped_mem));
		}*/
		
		void update(uint32_t index, GPU_T& vertex)
		{
			assert_cond(index * sizeof(GPU_T) < init_desc.ByteWidth, "out of range");

			//std::memcpy((GPU_T*)(mapped_mem.pData) + index, &vertex, sizeof(GPU_T));

			D3D11_BOX src_box = {};
			src_box.left = sizeof(GPU_T) * index;
			src_box.right = src_box.left + sizeof(GPU_T);
			src_box.top = 0;
			src_box.bottom = 1;
			src_box.front = 0;
			src_box.back = 1;

			ctx3->UpdateSubresource(buff, 0, &src_box, &vertex, 0, 0);
		}

		/*void set(uint32_t index, size_t field_offset, void* data, size_t field_size)
		{
			assert_cond((index * sizeof(GPU_T)) + field_offset < init_desc.ByteWidth, "out of range");

			std::memcpy((GPU_T*)(mapped_mem.pData).pData + index + field_offset, data, field_size);
		}*/

		// void get

		/*void unmap()
		{
			ctx3->Unmap(buff, 0);
			mapped_mem.pData = nullptr;
		}*/

		ID3D11Buffer* get()
		{
			return buff;
		}

		uint32_t size()
		{
			return init_desc.ByteWidth / sizeof(GPU_T);
		}

		~ArrayBuffer()
		{
			if (buff != nullptr) {
				buff->Release();
			}
		}
	};


	struct ConstantBufferField {
		size_t offset;
	};

	class ConstantBuffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ComPtr<ID3D11Buffer> buff;

	public:
		// Internal
		D3D11_MAPPED_SUBRESOURCE mapped_subres;
		size_t total_offset;
		std::vector<ConstantBufferField> fields;

		void _ensureCreateAndMapped();

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx,
			D3D11_USAGE usage, uint32_t cpu_access_flags);

		void addFloat();
		void addFloat2();
		void addFloat4();
		void addFloat4Array(uint32_t array_size);
	
		void setFloat(uint32_t field_idx, float value);
		void setFloat2(uint32_t field_idx, DirectX::XMFLOAT2& value);
		void setFloat4(uint32_t field_idx, float x, float y = 0, float z = 0, float w = 0);
		void setFloat4Array(uint32_t field_idx, uint32_t array_idx, DirectX::XMFLOAT4& value);
		void setFloat4Array(uint32_t field_idx, uint32_t array_idx, float x, float y = 0, float z = 0, float w = 0);

		ID3D11Buffer* get();
	};


	// TODO: StructuredBuffer


	class Texture {
	public:
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* ctx3;
		
		D3D11_TEXTURE2D_DESC tex_desc;
		ComPtr<ID3D11Texture2D> tex;
		
		// Mapped
		D3D11_MAPPED_SUBRESOURCE mapped;

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		ComPtr<ID3D11ShaderResourceView> srv;

		D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc;
		ComPtr<ID3D11DepthStencilView> dsv;

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		ComPtr<ID3D11RenderTargetView> rtv;

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_TEXTURE2D_DESC& desc);
		
		void createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
		void createDepthStencilView(D3D11_DEPTH_STENCIL_VIEW_DESC& desc);
		void createRenderTargetView(D3D11_RENDER_TARGET_VIEW_DESC& desc);

		void resize(uint32_t width, uint32_t height);
		void load(void* data);
		void load(void* data, uint32_t width, uint32_t height);

		void readbackAtPixel(uint32_t x, uint32_t y, uint32_t& r, uint32_t& g);

		void ensureUnmapped();

		// calls ensure unmapped
		ID3D11Texture2D* get();
		ID3D11ShaderResourceView* getSRV();
		ID3D11DepthStencilView* getDSV();
		ID3D11RenderTargetView* getRTV();
	};


	ErrStack singleLoad(ID3D11DeviceContext3* ctx, ID3D11Resource* resource,
		void* data, size_t load_size, uint32_t sub_resource_idx = 0);

	ErrStack resizeTexture2D(ID3D11Device5* dev, uint32_t new_width, uint32_t new_height,
		ComPtr<ID3D11Texture2D>& tex);

	void createVertexShaderFromPath(std::string path_to_file, ID3D11Device5* device,
		ID3D11VertexShader** r_vertex_shader,
		std::vector<char>* read_buffer = nullptr);

	void createPixelShaderFromPath(std::string path_to_file, ID3D11Device5* device,
		ID3D11PixelShader** r_pixel_shader,
		std::vector<char>* read_buffer = nullptr);


	class RasterizerState {
	public:
		ID3D11Device5* dev5;

		D3D11_RASTERIZER_DESC desc;
		ComPtr<ID3D11RasterizerState> rasterizer_state;

		bool has_new_state;

	public:
		void create(ID3D11Device5* dev, D3D11_RASTERIZER_DESC& desc);

		void setFillMode(D3D11_FILL_MODE fill_mode);

		void setCullMode(D3D11_CULL_MODE cull_mode);

		void setDepthBias(int32_t depth_bias);

		ID3D11RasterizerState* get();
	};


	struct IndexedDrawcallParams {
		uint32_t vertex_start_idx;
		uint32_t vertex_count;  // not required in drawcall but still usefull
		uint32_t index_start_idx;
		uint32_t index_count;
	};


	struct IndexedInstancedDrawcallParams {

	};
}
