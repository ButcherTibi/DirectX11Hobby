
// Header
#include "DX11Wrapper.hpp"

#include "FilePath.hpp"


void dx11::check(HRESULT result)
{
	if (result != S_OK) {
		__debugbreak();
	}
}

void dx11::Buffer::create(ID3D11Device5* device, ID3D11DeviceContext3* context, D3D11_BUFFER_DESC& desc)
{
	this->dev = device;
	this->ctx3 = context;
	this->init_desc = desc;
	this->buff = nullptr;
}

void dx11::Buffer::_ensureSize(size_t size)
{
	auto create_buff = [&]() {

		init_desc.ByteWidth = size;
		HRESULT hr = dev->CreateBuffer(&init_desc, NULL, buff.GetAddressOf());
		if (hr != S_OK) {
			throw DirectX11Exception(hr, "failed to create buffer");
		}
	};

	// Create
	if (buff == nullptr) {
		create_buff();
	}
	else {
		D3D11_BUFFER_DESC desc;
		buff->GetDesc(&desc);

		// Recreate
		if (size > desc.ByteWidth) {
			buff->Release();
			create_buff();
		}
	}
}

void dx11::Buffer::load(void* data, size_t load_size)
{
	_ensureSize(load_size);

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = ctx3->Map(buff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (hr != S_OK) {
		throw DirectX11Exception(hr, "failed to map buffer");
	}

	std::memcpy(mapped.pData, data, load_size);

	ctx3->Unmap(buff.Get(), 0);
}

void dx11::Buffer::beginLoad(size_t total_load_size, void*& mapped_mem)
{
	_ensureSize(total_load_size);

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = ctx3->Map(buff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	if (hr != S_OK) {
		throw DirectX11Exception(hr, "failed to map buffer");
	}

	mapped_mem = mapped.pData;
}

void dx11::Buffer::endLoad()
{
	ctx3->Unmap(buff.Get(), 0);
}

void dx11::Buffer::mapForWrite(size_t total_load_size, void*& mapped_mem)
{
	_ensureSize(total_load_size);

	D3D11_MAPPED_SUBRESOURCE mapped;
	HRESULT hr = ctx3->Map(buff.Get(), 0, D3D11_MAP_WRITE, 0, &mapped);
	if (hr != S_OK) {
		throw DirectX11Exception(hr, "failed to map buffer");
	}

	mapped_mem = mapped.pData;
}

void dx11::Buffer::unmap()
{
	ctx3->Unmap(buff.Get(), 0);
}

ID3D11Buffer* dx11::Buffer::get()
{
	return buff.Get();
}

size_t dx11::Buffer::getMemorySizeMegaBytes()
{
	D3D11_BUFFER_DESC desc;
	buff->GetDesc(&desc);

	return desc.ByteWidth / (1024LL * 1024);
}

void dx11::StagingBuffer::create(ID3D11Device5* device, ID3D11DeviceContext3* context)
{
	this->dev = device;
	this->ctx3 = context;

	this->init_desc = {};
	init_desc.Usage = D3D11_USAGE_STAGING;
	init_desc.CPUAccessFlags = D3D10_CPU_ACCESS_READ;

	this->mapped.pData = nullptr;
}

void dx11::StagingBuffer::resizeDiscard(size_t new_size)
{
	// fresh buffer
	if (buff == nullptr) {

		init_desc.ByteWidth = new_size;
		throwDX11(dev->CreateBuffer(&init_desc, NULL, &buff));
	}
	// resize but discard old
	else if (init_desc.ByteWidth < new_size) {

		ensureUnMapped();

		// destroy old
		buff->Release();

		// create new
		init_desc.ByteWidth = new_size;
		throwDX11(dev->CreateBuffer(&init_desc, NULL, &buff));
	}
}

void* dx11::StagingBuffer::dataReadOnly()
{
	if (mapped.pData == nullptr) {

		throwDX11(ctx3->Map(buff, 0, D3D11_MAP_READ, 0, &mapped));
	}

	return mapped.pData;
}

void dx11::StagingBuffer::ensureUnMapped()
{
	if (mapped.pData != nullptr) {

		ctx3->Unmap(buff, 0);
		mapped.pData = nullptr;
	}
}

ID3D11Buffer* dx11::StagingBuffer::get()
{
	ensureUnMapped();

	return buff;
}

dx11::StagingBuffer::~StagingBuffer()
{
	if (buff != nullptr) {
		buff->Release();
	}
}

void dx11::ConstantBuffer::_ensureCreateAndMapped()
{
	// ensure buffer is create
	if (buff == nullptr) {

		init_desc.ByteWidth = total_offset + 16 - total_offset % 16;  // round up, must be a multiple of 16
		throwDX11(dev->CreateBuffer(&init_desc, nullptr, buff.GetAddressOf()));
	}

	// ensure buffer is mapped
	if (mapped_subres.pData == nullptr) {

		throwDX11(ctx3->Map(buff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subres));
	}
}

void dx11::ConstantBuffer::create(ID3D11Device5* device, ID3D11DeviceContext3* context,
	D3D11_USAGE usage, uint32_t cpu_access_flags)
{
	dev = device;
	ctx3 = context;
	init_desc.Usage = usage;
	init_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	init_desc.CPUAccessFlags = cpu_access_flags;
	init_desc.MiscFlags = 0;
	init_desc.StructureByteStride = 0;

	buff = nullptr;
	total_offset = 0;
}

void dx11::ConstantBuffer::_add4BytesField(ConstantBufferField::FieldType type)
{
	ConstantBufferField& new_field = this->fields.emplace_back();
	new_field.type = type;
	new_field.offset = total_offset;

	total_offset = new_field.offset + 4;
}

void dx11::ConstantBuffer::addUint()
{
	_add4BytesField(ConstantBufferField::FieldType::UINT);
}

void dx11::ConstantBuffer::addInt()
{
	_add4BytesField(ConstantBufferField::FieldType::INT);
}

void dx11::ConstantBuffer::addFloat()
{
	_add4BytesField(ConstantBufferField::FieldType::FLOAT);
}

void dx11::ConstantBuffer::addFloat2()
{
	ConstantBufferField& new_field = this->fields.emplace_back();
	new_field.type = ConstantBufferField::FieldType::FLOAT2;

	size_t remainder = total_offset % 16;

	if (remainder >= 8) {
		new_field.offset = total_offset;
	}
	else {
		new_field.offset = total_offset + remainder;
	}

	total_offset = new_field.offset + 8;
}

void dx11::ConstantBuffer::addFloat4()
{
	ConstantBufferField& new_field = this->fields.emplace_back();
	new_field.type = ConstantBufferField::FieldType::FLOAT4;

	size_t remainder = total_offset % 16;

	if (remainder != 0) {
		new_field.offset = total_offset + remainder;
	}
	else {
		new_field.offset = total_offset;
	}

	total_offset = new_field.offset + 16;
}

void dx11::ConstantBuffer::addFloat4Array(uint32_t array_size)
{
	ConstantBufferField& new_field = this->fields.emplace_back();
	new_field.type = ConstantBufferField::FieldType::FLOAT_ARR;

	size_t padding = total_offset % 16;

	// arrays elements are always 16 bytes wide, so round up
	new_field.offset = total_offset + padding;

	total_offset = new_field.offset + 16 * array_size;
}

void dx11::ConstantBuffer::_set4BytesField(uint32_t field_idx, void* data, ConstantBufferField::FieldType type)
{
	assert_cond(fields[field_idx].type == type,
		"wrong field index, field type mismatch");

	_ensureCreateAndMapped();

	uint8_t* mem = ((uint8_t*)mapped_subres.pData) + fields[field_idx].offset;
	std::memcpy(mem, data, 4);
}

void dx11::ConstantBuffer::setUint(uint32_t field_idx, uint32_t value)
{
	_set4BytesField(field_idx, &value, ConstantBufferField::FieldType::UINT);
}

void dx11::ConstantBuffer::setInt(uint32_t field_idx, int32_t value)
{
	_set4BytesField(field_idx, &value, ConstantBufferField::FieldType::INT);
}

void dx11::ConstantBuffer::setFloat(uint32_t field_idx, float value)
{
	_set4BytesField(field_idx, &value, ConstantBufferField::FieldType::FLOAT);
}

void dx11::ConstantBuffer::setFloat2(uint32_t field_idx, DirectX::XMFLOAT2& value)
{
	assert_cond(fields[field_idx].type == ConstantBufferField::FieldType::FLOAT2,
		"wrong field index, field type mismatch");

	_ensureCreateAndMapped();

	uint8_t* mem = ((uint8_t*)mapped_subres.pData) + fields[field_idx].offset;
	std::memcpy(mem, &value, sizeof(DirectX::XMFLOAT2));
}

void dx11::ConstantBuffer::setFloat4(uint32_t field_idx, float x, float y, float z, float w)
{
	assert_cond(fields[field_idx].type == ConstantBufferField::FieldType::FLOAT4,
		"wrong field index, field type mismatch");

	_ensureCreateAndMapped();

	uint8_t* mem = ((uint8_t*)mapped_subres.pData) + fields[field_idx].offset;
	std::memcpy(mem, &x, 4);
	std::memcpy(mem + 4, &y, 4);
	std::memcpy(mem + 8, &z, 4);
	std::memcpy(mem + 12, &w, 4);
}

void dx11::ConstantBuffer::setFloat4Array(uint32_t field_idx, uint32_t array_idx, DirectX::XMFLOAT4& value)
{
	assert_cond(fields[field_idx].type == ConstantBufferField::FieldType::FLOAT_ARR,
		"wrong field index, field type mismatch");

	_ensureCreateAndMapped();

	uint8_t* mem = ((uint8_t*)mapped_subres.pData) + fields[field_idx].offset + 16 * array_idx;
	std::memcpy(mem, &value, sizeof(DirectX::XMFLOAT4));
}

void dx11::ConstantBuffer::setFloat4Array(uint32_t field_idx, uint32_t array_idx, float x, float y, float z, float w)
{
	assert_cond(fields[field_idx].type == ConstantBufferField::FieldType::FLOAT_ARR,
		"wrong field index, field type mismatch");

	_ensureCreateAndMapped();

	uint8_t* mem = ((uint8_t*)mapped_subres.pData) + fields[field_idx].offset + 16 * array_idx;
	std::memcpy(mem, &x, 4);
	std::memcpy(mem + 4, &y, 4);
	std::memcpy(mem + 8, &z, 4);
	std::memcpy(mem + 12, &w, 4);
}

ID3D11Buffer* dx11::ConstantBuffer::get()
{
	if (mapped_subres.pData != nullptr) {
		ctx3->Unmap(buff.Get(), 0);
		mapped_subres.pData = nullptr;
	}

	return buff.Get();
}

void dx11::Texture::create(ID3D11Device5* dev, ID3D11DeviceContext3* ctx, D3D11_TEXTURE2D_DESC& new_desc)
{
	this->dev5 = dev;
	this->ctx3 = ctx;
	this->tex_desc = new_desc;
	this->tex = nullptr;
	this->mapped.pData = nullptr;

	this->srv = nullptr;
	this->dsv = nullptr;
	this->rtv = nullptr;
}

void dx11::Texture::createShaderResourceView(D3D11_SHADER_RESOURCE_VIEW_DESC& desc)
{
	this->srv_desc = desc;
}

void dx11::Texture::createDepthStencilView(D3D11_DEPTH_STENCIL_VIEW_DESC& desc)
{
	this->dsv_desc = desc;
}

void dx11::Texture::createRenderTargetView(D3D11_RENDER_TARGET_VIEW_DESC& desc)
{
	this->rtv_desc = desc;
}

void dx11::Texture::resize(uint32_t width, uint32_t height)
{
	auto create_tex = [&]() {

		tex_desc.Width = width;
		tex_desc.Height = height;

		throwDX11(dev5->CreateTexture2D(&tex_desc, nullptr, tex.GetAddressOf()),
			"failed to create texture 2D");
	};

	if (tex == nullptr) {
		create_tex();
	}
	else if (tex_desc.Width != width || tex_desc.Height != height) {

		// child views are now invalid
		srv = nullptr;
		dsv = nullptr;
		rtv = nullptr;

		ensureUnmapped();
		tex->Release();
		create_tex();
	}
}

void dx11::Texture::load(void* data)
{
	size_t load_size = tex_desc.Width * tex_desc.Height;

	switch (tex_desc.Format) {
	case DXGI_FORMAT_R8_UNORM:
		// load_size *= 1;
		break;

	default:
		printf(code_location);
	}

	D3D11_MAPPED_SUBRESOURCE load_mapped;
	HRESULT hr = ctx3->Map(tex.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &load_mapped);
	if (hr != S_OK) {
		throw DirectX11Exception(hr, "failed to map texture");
	}

	std::memcpy(load_mapped.pData, data, load_size);

	ctx3->Unmap(tex.Get(), 0);
}

void dx11::Texture::load(void* data, uint32_t width, uint32_t height)
{
	auto create_tex_and_load = [&]() {

		tex_desc.Width = width;
		tex_desc.Height = height;

		size_t mem_pitch = tex_desc.Width;

		switch (tex_desc.Format) {
		case DXGI_FORMAT_R8_UNORM:
			// load_size *= 1;
			break;

		default:
			throw std::exception();
		}

		D3D11_SUBRESOURCE_DATA sub_data = {};
		sub_data.pSysMem = data;
		sub_data.SysMemPitch = mem_pitch;

		throwDX11(dev5->CreateTexture2D(&tex_desc, &sub_data, tex.GetAddressOf()),
			"failed to create texture 2D");
	};

	if (tex == nullptr) {
		create_tex_and_load();
	}
	else if (tex_desc.Width != width || tex_desc.Height != height) {

		tex->Release();
		create_tex_and_load();
	}
	else {
		load(data);
	}
}

void dx11::Texture::readPixel(uint32_t x, uint32_t y, uint32_t& r, uint32_t& g)
{
	if (mapped.pData == nullptr) {
		throwDX11(ctx3->Map(tex.Get(), 0, D3D11_MAP_READ, 0, &mapped));
	}

	uint32_t idx = (y * mapped.RowPitch) + (x * 8);

	std::array<uint32_t, 2> rg;
	std::memcpy(rg.data(), ((uint8_t*)mapped.pData) + idx, 8);

	r = rg[0];
	g = rg[1];
}

void dx11::Texture::readPixel(uint32_t x, uint32_t y, std::array<float, 4>& rgba)
{
	if (mapped.pData == nullptr) {
		throwDX11(ctx3->Map(tex.Get(), 0, D3D11_MAP_READ, 0, &mapped));
	}

	uint32_t stride = sizeof(uint32_t) * 4;
	uint32_t idx = (y * mapped.RowPitch) + (x * stride);

	std::memcpy(rgba.data(), ((uint8_t*)mapped.pData) + idx, stride);
}

void dx11::Texture::ensureUnmapped()
{
	if (mapped.pData != nullptr) {
		ctx3->Unmap(tex.Get(), 0);
		mapped.pData = nullptr;
	}
}

ID3D11Texture2D* dx11::Texture::get()
{
	if (tex == nullptr) {
		throwDX11(dev5->CreateTexture2D(&tex_desc, nullptr, tex.GetAddressOf()),
			"failed to create texture 2D");
	}

	ensureUnmapped();
	return tex.Get();
}

ID3D11ShaderResourceView* dx11::Texture::getSRV()
{
	if (srv == nullptr) {
		throwDX11(dev5->CreateShaderResourceView(tex.Get(), &srv_desc, srv.GetAddressOf()));
	}

	return srv.Get();
}

ID3D11DepthStencilView* dx11::Texture::getDSV()
{
	if (dsv == nullptr) {
		throwDX11(dev5->CreateDepthStencilView(tex.Get(), &dsv_desc, dsv.GetAddressOf()));
	}
	
	return dsv.Get();
}

ID3D11RenderTargetView* dx11::Texture::getRTV()
{
	if (rtv == nullptr) {
		throwDX11(dev5->CreateRenderTargetView(tex.Get(), &rtv_desc, rtv.GetAddressOf()));
	}

	return rtv.Get();
}

void dx11::createVertexShaderFromPath(std::string path_to_file, ID3D11Device5* device,
	ID3D11VertexShader** r_vertex_shader,
	std::vector<char>* read_buffer)
{
	std::vector<char> empty_vec;

	if (read_buffer == nullptr) {
		read_buffer = &empty_vec;
	}

	ErrStack err_stack = io::readLocalFile(path_to_file, *read_buffer);
	if (err_stack.isBad()) {
		throw WindowsException("failed to read compiled vertex shader code from path");
	}

	throwDX11(device->CreateVertexShader(read_buffer->data(), read_buffer->size(), nullptr,
		r_vertex_shader),
		"failed to create vertex shader");
}

void dx11::createPixelShaderFromPath(std::string path_to_file, ID3D11Device5* device,
	ID3D11PixelShader** r_pixel_shader,
	std::vector<char>* read_buffer)
{
	std::vector<char> empty_vec;

	if (read_buffer == nullptr) {
		read_buffer = &empty_vec;
	}

	ErrStack err_stack = io::readLocalFile(path_to_file, *read_buffer);
	if (err_stack.isBad()) {
		throw WindowsException("failed to read compiled pixel shader code from path");
	}

	throwDX11(device->CreatePixelShader(read_buffer->data(), read_buffer->size(), nullptr,
		r_pixel_shader),
		"failed to create pixel shader");
}

void dx11::createComputeShaderFromPath(std::string path_to_file, ID3D11Device5* device,
	ID3D11ComputeShader** r_compute_shader,
	std::vector<char>* read_buffer)
{
	std::vector<char> empty_vec;

	if (read_buffer == nullptr) {
		read_buffer = &empty_vec;
	}

	ErrStack err_stack = io::readLocalFile(path_to_file, *read_buffer);
	if (err_stack.isBad()) {
		throw WindowsException("failed to read compiled pixel shader code from path");
	}

	throwDX11(device->CreateComputeShader(read_buffer->data(), read_buffer->size(), nullptr,
		r_compute_shader),
		"failed to create compute shader");
}

void dx11::RasterizerState::create(ID3D11Device5* new_dev, D3D11_RASTERIZER_DESC& new_desc)
{
	this->dev5 = new_dev;
	this->desc = new_desc;

	this->has_new_state = true;
}

void dx11::RasterizerState::setFillMode(D3D11_FILL_MODE fill_mode)
{
	if (desc.FillMode != fill_mode) {
		desc.FillMode = fill_mode;
		has_new_state = true;
	}
}

void dx11::RasterizerState::setCullMode(D3D11_CULL_MODE cull_mode)
{
	if (desc.CullMode != cull_mode) {
		desc.CullMode = cull_mode;
		has_new_state = true;
	}
}

void dx11::RasterizerState::setDepthBias(int32_t depth_bias)
{
	if (desc.DepthBias != depth_bias) {
		desc.DepthBias = depth_bias;
		has_new_state = true;
	}
}

ID3D11RasterizerState* dx11::RasterizerState::get()
{
	if (has_new_state == true) {

		rasterizer_state = nullptr;
		throwDX11(dev5->CreateRasterizerState(&desc, rasterizer_state.GetAddressOf()));

		this->has_new_state = false;
	}
	
	return rasterizer_state.Get();
}
