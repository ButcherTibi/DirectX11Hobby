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

		ComPtr<ID3D11Buffer> buff = nullptr;

	public:
		void _ensureSize(size_t size);

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_BUFFER_DESC& desc);

		void load(void* data, size_t load_size);

		void beginLoad(size_t total_load_size, void*& mapped_buffer);

		void endLoad();

		ID3D11Buffer* get();

		size_t getMemorySizeMegaBytes();
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


	class Texture {
	public:
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* ctx3;
		
		D3D11_TEXTURE2D_DESC tex_desc;
		ComPtr<ID3D11Texture2D> tex = nullptr;
		
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		ComPtr<ID3D11ShaderResourceView> srv;

		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		ComPtr<ID3D11RenderTargetView> rtv;

	public:
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_TEXTURE2D_DESC& desc);
		
		void createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC& desc);
		void createRenderTargetView(D3D11_RENDER_TARGET_VIEW_DESC& desc);

		void resize(uint32_t width, uint32_t height);
		void load(void* data);
		void load(void* data, uint32_t width, uint32_t height);

		ID3D11Texture2D* get();
		ID3D11ShaderResourceView* getSRV();
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


	class FrameAttachmentsSelection {
		// set textures
		// set rtv
		// set srv
		// set dtex
		// set dsv
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
