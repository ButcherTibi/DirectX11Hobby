#pragma once

// Standard
#include <vector>
#include <array>

// DirectX 11
#include <d3d11_4.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>

// ComPtr
#include <wrl\client.h>

// Error
#include "ErrorStack.hpp"

#include <ButchersToolbox/Filesys/Filesys.hpp>


template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

namespace dx11 {

	void check(HRESULT result);

	class Buffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ComPtr<ID3D11Buffer> buff = nullptr;

	public:
		void _ensureSize(size_t size);

	public:
		[[deprecated]]
		void create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_BUFFER_DESC& desc);

		void load(void* data, size_t load_size);

		void beginLoad(size_t total_load_size, void*& mapped_mem);
		void endLoad();

		void mapForWrite(size_t total_load_size, void*& mapped_mem);
		void unmap();

		ID3D11Buffer* get();

		size_t getMemorySizeMegaBytes();
	};


	class StagingBuffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ID3D11Buffer* buff = nullptr;
		D3D11_MAPPED_SUBRESOURCE mapped;

	public:
		void create(ID3D11Device5* device, ID3D11DeviceContext3* context);

		void resizeDiscard(size_t new_size);

		void* dataReadOnly();

		void ensureUnMapped();

		ID3D11Buffer* get();

		~StagingBuffer();
	};


	// designed to hold typed arrays of gpu data
	template<typename GPU_T>
	class ArrayBuffer {
	public:
		ID3D11Device5* dev;
		ID3D11DeviceContext3* ctx3;
		D3D11_BUFFER_DESC init_desc;

		ID3D11Buffer* buff = nullptr;

		D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
		ComPtr<ID3D11UnorderedAccessView> uav;

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		ComPtr<ID3D11ShaderResourceView> srv;

		D3D11_MAPPED_SUBRESOURCE mapped_mem;
		uint32_t _count;

	public:
		void create(ID3D11Device5* device, ID3D11DeviceContext3* context, D3D11_BUFFER_DESC& desc)
		{
			this->dev = device;
			this->ctx3 = context;

			if ((desc.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) == D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) {
				desc.StructureByteStride = sizeof(GPU_T);
			}

			this->init_desc = desc;

			this->uav_desc.Buffer.NumElements = 0;
			this->uav = nullptr;

			this->srv_desc.Buffer.NumElements = 0;
			this->srv = nullptr;

			this->mapped_mem.pData = nullptr;
			this->_count = 0;
		}

		// creates a new buffer, copying the data to the new buffer
		// size is the number of elements
		void resize(uint32_t new_count)
		{
			uint32_t new_size_bytes = new_count * sizeof(GPU_T);

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

				// destroy old
				srv = nullptr;
				uav = nullptr;
				buff->Release();

				// assign/create new
				buff = new_buff;

				init_desc.ByteWidth = new_size_bytes;
			}

			this->_count = new_count;
		}

		// creates a new buffer, discarding the old data
		void resizeDiscard(uint32_t new_count)
		{
			uint32_t load_size = new_count * sizeof(GPU_T);

			// fresh buffer
			if (buff == nullptr) {

				init_desc.ByteWidth = load_size;
				throwDX11(dev->CreateBuffer(&init_desc, NULL, &buff));
			}
			// resize but discard old
			else if (init_desc.ByteWidth < load_size) {

				// destroy old
				srv = nullptr;
				uav = nullptr;
				buff->Release();

				// create new
				init_desc.ByteWidth = load_size;
				throwDX11(dev->CreateBuffer(&init_desc, NULL, &buff));
			}

			this->_count = new_count;
		}

		// upload entire vector from the start
		// resizes the destination buffer to fit all data
		void upload(std::vector<GPU_T>& src, uint32_t start_idx = 0)
		{
			resizeDiscard(start_idx + src.size());

			D3D11_BOX dest_box = {};
			dest_box.left = sizeof(GPU_T) * start_idx;
			dest_box.right = dest_box.left + sizeof(GPU_T) * src.size();
			dest_box.top = 0;
			dest_box.bottom = 1;
			dest_box.front = 0;
			dest_box.back = 1;

			assert_cond(dest_box.left < dest_box.right);

			ctx3->UpdateSubresource(buff, 0, &dest_box,
				src.data(), 0, 0);
		}
		
		// upload change to the buffer at the specified index
		// ideally it would be operator[] but I want to take a reference not give one
		void upload(uint32_t index, GPU_T& vertex)
		{
			assert_cond(index * sizeof(GPU_T) < init_desc.ByteWidth, "index out of range");

			D3D11_BOX dest_box = {};
			dest_box.left = sizeof(GPU_T) * index;
			dest_box.right = dest_box.left + sizeof(GPU_T);
			dest_box.top = 0;
			dest_box.bottom = 1;
			dest_box.front = 0;
			dest_box.back = 1;

			ctx3->UpdateSubresource(buff, 0, &dest_box,
				&vertex, 0, 0);
		}

		void* dataReadOnly()
		{
			if (mapped_mem.pData == nullptr) {
				throwDX11(ctx3->Map(buff, 0, D3D11_MAP_READ, 0, &mapped_mem));
			}

			return mapped_mem.pData;
		}

		void ensureUnMapped()
		{
			if (mapped_mem.pData != nullptr) {
				ctx3->Unmap(buff, 0);
				mapped_mem.pData = nullptr;
			}
		}

		// download dest.size() elemnent count from buffer
		// staging buffer may be resized to fit content
		void download(std::vector<GPU_T>& dest, StagingBuffer& staging)
		{
			assert_cond(dest.size() <= count(), "request to download more than buffer size");

			staging.resizeDiscard(sizeof(GPU_T) *dest.size());

			D3D11_BOX box = {};
			box.top = 0;
			box.bottom = 1;
			box.left = 0;
			box.right = sizeof(GPU_T) * dest.size();
			box.front = 0;
			box.back = 1;

			ctx3->CopySubresourceRegion(
				staging.get(), 0,
				0, 0, 0,
				buff, 0,
				&box
			);

			std::memcpy(dest.data(), staging.dataReadOnly(), sizeof(GPU_T) * dest.size());
			staging.ensureUnMapped();
		}
		
		ID3D11Buffer* get()
		{
			return buff;
		}

		// recreates the UAV if buffer changed and returns it
		ID3D11UnorderedAccessView* getUAV()
		{
			if (uav == nullptr) {

				uav_desc.Format = DXGI_FORMAT_UNKNOWN;
				uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
				uav_desc.Buffer.FirstElement = 0;
				uav_desc.Buffer.NumElements = count();
				uav_desc.Buffer.Flags = 0;

				throwDX11(dev->CreateUnorderedAccessView(buff, &uav_desc, uav.GetAddressOf()));
			}
			return uav.Get();
		}

		// recreates the SRV if buffer changed and returns it
		ID3D11ShaderResourceView* getSRV()
		{
			if (srv == nullptr) {

				srv_desc.Format = DXGI_FORMAT_UNKNOWN;
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
				srv_desc.Buffer.FirstElement = 0;
				srv_desc.Buffer.NumElements = count();

				throwDX11(dev->CreateShaderResourceView(buff, &srv_desc, srv.GetAddressOf()));
			}

			return srv.Get();
		}

		// get the number of elements
		uint32_t capacity()
		{
			return init_desc.ByteWidth / sizeof(GPU_T);
		}

		uint32_t count()
		{
			return this->_count;
		}

		// get the size of the buffer in bytes
		size_t memSizeBytes()
		{
			return init_desc.ByteWidth;
		}

		void deallocate()
		{
			ensureUnMapped();

			uav = nullptr;
			srv = nullptr;

			if (buff != nullptr) {

				buff->Release();
				buff = nullptr;  // to know when to recreate in resize methods
			}

			_count = 0;
		}

		~ArrayBuffer()
		{
			if (buff != nullptr) {
				buff->Release();
			}
		}
	};


	struct ConstantBufferField {
		enum class FieldType {
			UINT,
			INT,

			FLOAT,
			FLOAT2,
			FLOAT4,
			FLOAT_ARR
		};

		FieldType type;
		size_t offset;
	};


	// designed to wrap usage and declaration of constant buffer
	// allows creating the layout of the constant buffer one field at a time
	// with this calculating that DISGUSTING alignment is no longer necesary
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


		// each method adds a field to the buffer structure, calculating the offset required
		// the order of addition must match with the field_idx
		// Example:
		//   addUint();
		//   addFloat();
		//
		//   setUint(1, 1234);  // bad uint was added first
		//   setFloat(0, 1234.f);

		void _add4BytesField(ConstantBufferField::FieldType type);
		void addUint();
		void addInt();
		void addFloat();

		void addFloat2();
		void addFloat4();
		void addFloat4Array(uint32_t array_size);
	
		void _set4BytesField(uint32_t field_idx, void* data, ConstantBufferField::FieldType type);
		void setUint(uint32_t field_idx, uint32_t value);
		void setInt(uint32_t field_idx, int32_t value);
		void setFloat(uint32_t field_idx, float value);

		void setFloat2(uint32_t field_idx, DirectX::XMFLOAT2& value);
		void setFloat4(uint32_t field_idx, float x, float y = 0, float z = 0, float w = 0);
		void setFloat4Array(uint32_t field_idx, uint32_t array_idx, DirectX::XMFLOAT4& value);
		void setFloat4Array(uint32_t field_idx, uint32_t array_idx, float x, float y = 0, float z = 0, float w = 0);

		ID3D11Buffer* get();
	};


	template<typename GPU_UP_T>
	class ComputeCall {
	public:
		ID3D11DeviceContext3* ctx;

		std::vector<GPU_UP_T> uploads;
		ArrayBuffer<GPU_UP_T> gpu_uploads;

		ComPtr<ID3D11ComputeShader> shader;

	public:
		void create(ID3D11Device5* device, ID3D11DeviceContext3* context,
			std::string shader_path, std::vector<char>* read_buffer)
		{
			this->ctx = context;

			D3D11_BUFFER_DESC desc = {};
			desc.Usage = D3D11_USAGE_DEFAULT;
			desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

			this->gpu_uploads.create(device, context, desc);

			createComputeShaderFromPath(shader_path, device,
				this->shader.GetAddressOf(), read_buffer);
		}

		std::vector<GPU_UP_T>& getUploadData()
		{
			return uploads;
		}

		void call(ConstantBuffer* constant_buffer,
			std::vector<ID3D11ShaderResourceView*> srvs,
			std::vector<ID3D11UnorderedAccessView*> uavs,
			uint32_t dispatch_x = 0, uint32_t dispatch_y = 1, uint32_t dispatch_z = 1)
		{
			// Uploads
			this->gpu_uploads.upload(uploads);

			// Commands
			ctx->ClearState();

			if (constant_buffer != nullptr) {
				ctx->CSSetConstantBuffers(0, &constant_buffer->get(), 1);
			}

			// Shader Resource Views
			srvs.insert(srvs.begin(), gpu_uploads.getSRV());
			ctx->CSSetShaderResources(0, srvs.size(), srvs.data());

			if (uavs.size()) {
				ctx->CSSetUnorderedAccessViews(0, uavs.size(), uavs.data(), nullptr);
			}

			ctx->CSSetShader(shader.Get(), nullptr, 0);

			if (dispatch_x == 0) {
				ctx->Dispatch(gpu_uploads.count(), dispatch_y, dispatch_z);
			}
			else {
				ctx->Dispatch(dispatch_x, dispatch_y, dispatch_z);
			}
		}
	};


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

		void readPixel(uint32_t x, uint32_t y, uint32_t& r, uint32_t& g);
		void readPixel(uint32_t x, uint32_t y, std::array<float, 4>& rgba);

		void ensureUnmapped();

		// calls ensure unmapped
		ID3D11Texture2D* get();
		ID3D11ShaderResourceView* getSRV();
		ID3D11DepthStencilView* getDSV();
		ID3D11RenderTargetView* getRTV();
	};


	void createVertexShaderFromPath(std::string path_to_file, ID3D11Device5* device,
		ID3D11VertexShader** r_vertex_shader,
		std::vector<char>* read_buffer = nullptr);

	void createPixelShaderFromPath(std::string path_to_file, ID3D11Device5* device,
		ID3D11PixelShader** r_pixel_shader,
		std::vector<char>* read_buffer = nullptr);

	void createComputeShaderFromPath(std::string path_to_file, ID3D11Device5* device,
		ID3D11ComputeShader** r_compute_shader,
		std::vector<char>* read_buffer = nullptr);


	template<typename DX11_SHADER_T = ID3D11ComputeShader>
	class Shader {
		ID3D11Device5* dev;

		std::string source_code_path;
		std::vector<uint8_t> compiled_shader;
		ComPtr<DX11_SHADER_T> shader = nullptr;

	public:
		inline static filesys::Path<char> shaders_folder;
		inline static filesys::Path<char> compiled_shaders_folder;

	private:
		void createShader()
		{
			HRESULT hr;

			if constexpr (std::is_same<DX11_SHADER_T, ID3D11VertexShader>()) {
				hr = dev->CreateVertexShader(compiled_shader.data(), compiled_shader.size(), nullptr,
					shader.GetAddressOf());
			}
			else if constexpr (std::is_same<DX11_SHADER_T, ID3D11PixelShader>()) {
				hr = dev->CreatePixelShader(compiled_shader.data(), compiled_shader.size(), nullptr,
					shader.GetAddressOf());
			}
			else if constexpr (std::is_same<DX11_SHADER_T, ID3D11GeometryShader>()) {
				hr = dev->CreateGeometryShader(compiled_shader.data(), compiled_shader.size(), nullptr,
					shader.GetAddressOf());
			}
			else if constexpr (std::is_same<DX11_SHADER_T, ID3D11ComputeShader>()) {
				hr = dev->CreateComputeShader(compiled_shader.data(), compiled_shader.size(), nullptr,
					shader.GetAddressOf());
			}

			if (hr != S_OK) {
				__debugbreak();
			}
		}

	public:
		void create(ID3D11Device5* device,
			std::string relative_source_code_path,
			std::string relative_compiled_shader_path)
		{
			this->dev = device;

			auto path = shaders_folder;
			path.append(relative_source_code_path);
			source_code_path = path.toString();

			path = compiled_shaders_folder;
			path.append(relative_compiled_shader_path);

			filesys::File<char>::read(path.toString(), compiled_shader);

			createShader();
		}

		DX11_SHADER_T* Get()
		{
			return shader.Get();
		}
	};


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
}
