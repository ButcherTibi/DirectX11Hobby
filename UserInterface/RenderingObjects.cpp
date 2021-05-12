
// Header
#include "RenderingObjects.hpp"

#include "NuiLibrary.hpp"
#include "CharacterAtlas.hpp"
#include "BasicMath.h"


using namespace nui;

//
//void RectGradientRender::init(Window* win)
//{
//	this->window = win;
//}
//
//void RectGradientRender::addInstance(RectGradientInstance& props)
//{
//
//}
//
//void RectGradientRender::generateGPU_Data(RectGradientRenderProps& props)
//{
//	Window* w = window;
//
//	coloring = props.coloring;
//
//	vertex_start_idx = w->gradient_verts_idx;
//	index_start_idx = w->indexes_idx;
//
//	// Top Left
//	GPU_SimpleVertex* vertex = &window->gradient_verts[w->gradient_verts_idx];
//	vertex->pos = toXM(0, 0);
//
//	// Top Right
//	vertex = &w->gradient_verts[w->gradient_verts_idx + 1];
//	vertex->pos = toXM(props.size[0], 0);
//
//	// Bot Right
//	vertex = &w->gradient_verts[w->gradient_verts_idx + 2];
//	vertex->pos = toXM(props.size[0], props.size[1]);
//
//	// Bot Left
//	vertex = &w->gradient_verts[w->gradient_verts_idx + 3];
//	vertex->pos = toXM(0, props.size[1]);
//
//	// Tesselation 0 to 2
//	w->indexes[w->indexes_idx + 0] = w->gradient_verts_idx + 0;
//	w->indexes[w->indexes_idx + 1] = w->gradient_verts_idx + 1;
//	w->indexes[w->indexes_idx + 2] = w->gradient_verts_idx + 2;
//
//	w->indexes[w->indexes_idx + 3] = w->gradient_verts_idx + 2;
//	w->indexes[w->indexes_idx + 4] = w->gradient_verts_idx + 3;
//	w->indexes[w->indexes_idx + 5] = w->gradient_verts_idx + 0;
//
//	// Rect Instance
//	screen_pos[0] = props.screen_pos[0];
//	screen_pos[1] = props.screen_pos[1];
//
//	size[0] = props.size[0];
//	size[1] = props.size[1];
//
//	switch (props.coloring) {
//	case BackgroundColoring::FLAT_FILL: {
//
//		assert_cond(props.colors.size() > 0, "no colors specified");
//
//		colors[0] = toXM(props.colors[0].color.rgba);
//		break;
//	}
//
//	case BackgroundColoring::LINEAR_GRADIENT: {
//
//		assert_cond(props.colors.size() > 0, "no colors specified");
//
//		for (uint32_t i = 0; i < colors.size(); i++) {
//			colors[i] = toXM(props.colors[i].color.rgba);
//			color_lenghts[i] = props.colors[i].pos;
//		}
//
//		gradient_angle = toRadians(gradient_angle);
//		break;
//	}
//	}
//
//	w->gradient_verts_idx += 4;
//	w->indexes_idx += 6;
//
//	// Drawcall
//	vertex_count = 6;
//	index_count = 6;
//}
//
//void RectGradientRender::draw()
//{
//	Instance* instance = window->instance;
//	ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();
//
//	// Buffer
//	{
//		window->gradient_rect_cbuff.setFloat4(GPU_RectDrawcallFields::POSITION_SIZE,
//			(float)screen_pos[0], (float)screen_pos[1], (float)size[0], (float)size[1]);
//
//		for (uint32_t i = 0; i < 8; i++) {
//			window->gradient_rect_cbuff.setFloat4Array(GPU_RectDrawcallFields::COLORS, i, colors[i]);
//			window->gradient_rect_cbuff.setFloat4Array(GPU_RectDrawcallFields::COLOR_LENGHTS, i, color_lenghts[i]);
//		}
//
//		window->gradient_rect_cbuff.setFloat(GPU_RectDrawcallFields::GRADIENT_ANGLE, gradient_angle);
//	}
//
//	// Input Assembly
//	{
//		im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//		im_ctx3->IASetInputLayout(instance->simple_input_layout.Get());
//
//		std::array<ID3D11Buffer*, 1> vertex_buffs = {
//			window->gradient_vbuff.get()
//		};
//		std::array<uint32_t, 1> strides = {
//			sizeof(GPU_SimpleVertex)
//		};
//		std::array<uint32_t, 1> offsets = {
//			0
//		};
//		im_ctx3->IASetVertexBuffers(0, vertex_buffs.size(), vertex_buffs.data(), strides.data(), offsets.data());
//	}
//
//	// Vertex Shader
//	{
//		std::array<ID3D11Buffer*, 1> vs_cbuffs = {
//			window->cbuff.get()
//		};
//		im_ctx3->VSSetConstantBuffers(0, vs_cbuffs.size(), vs_cbuffs.data());
//
//		im_ctx3->VSSetShader(instance->simple_vs.Get(), nullptr, 0);
//	}
//
//	// Rasterizer
//	{
//		im_ctx3->RSSetViewports(1, &window->viewport);
//		im_ctx3->RSSetState(instance->solid_back_rs.Get());
//	}
//
//	// Pixel Shader
//	{
//		std::array<ID3D11Buffer*, 1> ps_cbuffs = {
//			window->gradient_rect_cbuff.get()
//		};
//		im_ctx3->PSSetConstantBuffers(0, ps_cbuffs.size(), ps_cbuffs.data());
//
//		switch (coloring) {
//		case BackgroundColoring::FLAT_FILL: {
//			im_ctx3->PSSetShader(instance->rect_flat_fill_ps.Get(), nullptr, 0);
//			break;
//		}
//		case BackgroundColoring::LINEAR_GRADIENT: {
//			im_ctx3->PSSetShader(instance->rect_gradient_linear_ps.Get(), nullptr, 0);
//			break;
//		}
//		}
//	}
//	
//	// Output Merger
//	{
//		std::array<float, 4> blend_factor = {
//			0, 0, 0, 0
//		};
//		im_ctx3->OMSetBlendState(instance->blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);
//
//		std::array<ID3D11RenderTargetView*, 1> srv = {
//			window->present_rtv.Get()
//		};
//		im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
//	}
//	
//
//	// Draw
//	im_ctx3->DrawIndexed(index_count, index_start_idx, 0);
//}

void RectRender::init(Window* win)
{
	this->window = win;

	Instance* instance = win->instance;

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		vbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		idxbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(GPU_RectInstance);

		sbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}
}

void RectRender::reset()
{
	this->instances_props.clear();
}

void RectRender::addInstance(RectInstance& props)
{
	this->instances_props.push_back(props);
}

void RectRender::generateGPU_Data()
{
	if (instances_props.size() == 0) {
		return;
	}

	verts.resize(instances_props.size() * 4);
	indexes.resize(instances_props.size() * 6);
	instances.resize(instances_props.size());

	uint32_t vertex_idx = 0;
	uint32_t index_idx = 0;
	for (uint32_t i = 0; i < instances_props.size(); i++) {

		RectInstance& props = instances_props[i];

		// Vertices
		GPU_SimpleVertex& tl_v = verts[vertex_idx];
		GPU_SimpleVertex& tr_v = verts[vertex_idx + 1];
		GPU_SimpleVertex& br_v = verts[vertex_idx + 2];
		GPU_SimpleVertex& bl_v = verts[vertex_idx + 3];

		glm::ivec2 pos = { props.screen_pos[0], props.screen_pos[1] };
		tl_v.pos = toXM(pos);
		tr_v.pos = toXM(pos.x + props.size[0], pos.y);
		br_v.pos = toXM(pos.x + props.size[0], pos.y + props.size[1]);;
		bl_v.pos = toXM(pos.x, pos.y + props.size[1]);
		tl_v.instance_id = i;
		tr_v.instance_id = i;
		br_v.instance_id = i;
		bl_v.instance_id = i;

		// Indexes
		indexes[index_idx + 0] = vertex_idx + 0;
		indexes[index_idx + 1] = vertex_idx + 1;
		indexes[index_idx + 2] = vertex_idx + 2;

		indexes[index_idx + 3] = vertex_idx + 2;
		indexes[index_idx + 4] = vertex_idx + 3;
		indexes[index_idx + 5] = vertex_idx + 0;

		// Instance
		GPU_RectInstance& instance = instances[i];
		instance.color = toXM(props.color.rgba);

		vertex_idx += 4;
		index_idx += 6;
	}
}

void RectRender::draw()
{
	if (instances_props.size() == 0) {
		return;
	}

	Instance* instance = window->instance;
	ID3D11Device5* dev5 = instance->dev5.Get();
	ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();

	{
		vbuff.load(verts.data(), sizeof(GPU_SimpleVertex) * verts.size());
		idxbuff.load(indexes.data(), sizeof(uint32_t) * indexes.size());
		sbuff.load(instances.data(), sizeof(GPU_RectInstance) * instances.size());

		sbuff_srv = nullptr;

		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = instances.size();

		throwDX11(dev5->CreateShaderResourceView(sbuff.get(), &desc, sbuff_srv.GetAddressOf()));
	}

	// Input Assembly
	{
		im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		im_ctx3->IASetInputLayout(instance->simple_input_layout.Get());

		std::array<ID3D11Buffer*, 1> vertex_buffs = {
			vbuff.get()
		};
		std::array<uint32_t, 1> strides = {
			sizeof(GPU_SimpleVertex)
		};
		std::array<uint32_t, 1> offsets = {
			0
		};
		im_ctx3->IASetVertexBuffers(0, vertex_buffs.size(), vertex_buffs.data(), strides.data(), offsets.data());

		im_ctx3->IASetIndexBuffer(idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);
	}

	// Vertex Shader
	{
		std::array<ID3D11Buffer*, 1> vs_cbuffs = {
			window->cbuff.get()
		};
		im_ctx3->VSSetConstantBuffers(0, vs_cbuffs.size(), vs_cbuffs.data());

		im_ctx3->VSSetShader(instance->simple_vs.Get(), nullptr, 0);
	}

	// Rasterizer
	{
		im_ctx3->RSSetViewports(1, &window->viewport);
		im_ctx3->RSSetState(instance->solid_back_rs.Get());
	}

	// Pixel Shader
	{	
		std::array<ID3D11ShaderResourceView*, 1> ps_srv = {
			sbuff_srv.Get()
		};
		im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());
	
		im_ctx3->PSSetShader(instance->rect_ps.Get(), nullptr, 0);
	}

	// Output Merger
	{
		std::array<float, 4> blend_factor = {
			0, 0, 0, 0
		};
		im_ctx3->OMSetBlendState(instance->blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

		std::array<ID3D11RenderTargetView*, 1> srv = {
			window->present_rtv.Get()
		};
		im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
	}

	im_ctx3->DrawIndexedInstanced(indexes.size(), 1,
		0, 0, 0);
}

void TextRender::init(Window* win)
{
	this->window = win;

	Instance* instance = win->instance;

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		vbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		idxbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}

	{
		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = sizeof(GPU_TextInstance);

		sbuff.create(instance->dev5.Get(), instance->im_ctx3.Get(), desc);
	}
}

void TextRender::reset()
{
	instances_props.clear();
}

void TextRender::addInstance(TextInstance& props, uint32_t& r_width, uint32_t& r_height)
{
	InstanceProps& instance = instances_props.emplace_back();
	auto& chars = instance.chars;
	chars.resize(props.text.size());

	FontSize* font_size;
	{
		nui::CharacterAtlas& atlas = window->instance->char_atlas;
		atlas.ensureFontWithSize(props.font_family, props.font_style, props.font_size, font_size);
	}

	r_width = 0;

	// if line height is unspecified use default from font file
	uint32_t line_height;
	if (props.line_height) {
		line_height = props.line_height;
	}
	else {
		line_height = font_size->ascender;
	}

	glm::ivec2 pen = { props.pos[0], props.pos[1] };
	pen.y += line_height;

	uint32_t i = 0;;
	for (uint32_t unicode : props.text) {

		switch (unicode) {
			// New Line
		case 0x000A: {
			pen.x = 0;  // Carriage Return
			pen.y += line_height;  // Line Feed
			break;
		}

		default: {
			Character* chara = font_size->findCharacter(unicode);
			chars[i].pos = { pen.x, pen.y };
			chars[i].chara = chara;

			// Move the writing pen to the next character
			pen.x += chara->advance_X;

			uint32_t new_width = pen.x - props.pos[0];

			if (new_width > r_width) {
				r_width = new_width;
			}
		}
		}

		i++;
	}

	r_height = (pen.y + font_size->descender) - props.pos[1];

	instance.color = props.color;
}

void TextRender::generateGPU_Data()
{
	if (instances_props.size() == 0) {
		return;
	}

	{
		uint32_t vertex_count = 0;
		uint32_t index_count = 0;
		uint32_t instance_count = 0;

		for (InstanceProps& instance_prop : instances_props) {
			for (auto& positioned_char : instance_prop.chars) {

				if (positioned_char.chara->zone != nullptr) {
					vertex_count += 4;
					index_count += 6;
				}
			}

			instance_count++;
		}

		verts.resize(vertex_count);
		indexes.resize(index_count);
		instances.resize(instance_count);
	}

	uint32_t vertex_idx = 0;
	uint32_t index_idx = 0;
	uint32_t instance_idx = 0;
	for (InstanceProps& instance_prop : instances_props) {

		auto& chars = instance_prop.chars;

		for (auto& positioned_char : chars) {

			Character* chara = positioned_char.chara;

			if (chara->zone != nullptr) {

				uint32_t bitmap_width = chara->zone->bb_pix.getWidth();
				uint32_t bitmap_height = chara->zone->bb_pix.getHeight();

				int32_t char_top = chara->bitmap_top;

				glm::ivec2 character_pos = { positioned_char.pos[0], positioned_char.pos[1] };
				character_pos.x += chara->bitmap_left;
				character_pos.y += bitmap_height - char_top;

				// Top Left
				glm::ivec2 pos = character_pos;
				pos.y -= bitmap_height;

				GPU_CharacterVertex* v = &verts[vertex_idx];
				v->pos = toXM(pos);
				v->uv.x = chara->zone->bb_uv.x0;
				v->uv.y = chara->zone->bb_uv.y0;
				v->instance_id = instance_idx;

				// Top Right
				pos = character_pos;
				pos.x += bitmap_width;
				pos.y -= bitmap_height;

				v = &verts[vertex_idx + 1];
				v->pos = toXM(pos);
				v->uv.x = chara->zone->bb_uv.x1;
				v->uv.y = chara->zone->bb_uv.y0;
				v->instance_id = instance_idx;

				// Bot Right
				pos = character_pos;
				pos.x += bitmap_width;

				v = &verts[vertex_idx + 2];
				v->pos = toXM(pos);
				v->uv.x = chara->zone->bb_uv.x1;
				v->uv.y = chara->zone->bb_uv.y1;
				v->instance_id = instance_idx;

				// Bot Left
				pos = character_pos;

				v = &verts[vertex_idx + 3];
				v->pos = toXM(pos);
				v->uv.x = chara->zone->bb_uv.x0;
				v->uv.y = chara->zone->bb_uv.y1;
				v->instance_id = instance_idx;

				// Tesselation 0 to 2
				indexes[index_idx + 0] = vertex_idx + 0;
				indexes[index_idx + 1] = vertex_idx + 1;
				indexes[index_idx + 2] = vertex_idx + 2;

				indexes[index_idx + 3] = vertex_idx + 2;
				indexes[index_idx + 4] = vertex_idx + 3;
				indexes[index_idx + 5] = vertex_idx + 0;

				vertex_idx += 4;
				index_idx += 6;
			}
		}

		// Text Instance
		GPU_TextInstance& tex_inst = instances[instance_idx];
		tex_inst.color = toXM(instance_prop.color.rgba);

		instance_idx++;
	}
}

void TextRender::draw()
{
	if (instances_props.size() == 0) {
		return;
	}

	Instance* instance = window->instance;
	ID3D11Device5* dev5 = instance->dev5.Get();
	ID3D11DeviceContext3* im_ctx3 = instance->im_ctx3.Get();

	{
		vbuff.load(verts.data(), sizeof(GPU_CharacterVertex) * verts.size());
		idxbuff.load(indexes.data(), sizeof(uint32_t) * indexes.size());
		sbuff.load(instances.data(), sizeof(GPU_TextInstance) * instances.size());

		sbuff_srv = nullptr;

		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		desc.Buffer.FirstElement = 0;
		desc.Buffer.NumElements = instances.size();

		throwDX11(dev5->CreateShaderResourceView(sbuff.get(), &desc, sbuff_srv.GetAddressOf()));
	}

	// Input Assembly
	{
		im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		im_ctx3->IASetInputLayout(instance->char_input_layout.Get());

		std::array<ID3D11Buffer*, 1> vertex_buffs = {
			vbuff.get(),
		};
		std::array<uint32_t, 1> strides = {
			sizeof(GPU_CharacterVertex)
		};
		std::array<uint32_t, 1> offsets = {
			0
		};
		im_ctx3->IASetVertexBuffers(0, vertex_buffs.size(), vertex_buffs.data(), strides.data(), offsets.data());
	
		im_ctx3->IASetIndexBuffer(idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);
	}

	// Vertex Shader
	{
		std::array<ID3D11Buffer*, 1> vs_cbuffs = {
			window->cbuff.get()
		};

		im_ctx3->VSSetConstantBuffers(0, vs_cbuffs.size(), vs_cbuffs.data());

		im_ctx3->VSSetShader(instance->char_vs.Get(), nullptr, 0);
	}

	// Rasterizer
	{
		im_ctx3->RSSetViewports(1, &window->viewport);
		im_ctx3->RSSetState(instance->solid_back_rs.Get());
	}

	// Pixel Shader
	{
		std::array<ID3D11SamplerState*, 1> samplers = {
			instance->char_atlas_sampler.Get()
		};
		im_ctx3->PSSetSamplers(0, samplers.size(), samplers.data());

		std::array<ID3D11ShaderResourceView*, 2> ps_srv = {
			instance->char_atlas_tex.getSRV(), sbuff_srv.Get()
		};
		im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());

		im_ctx3->PSSetShader(instance->char_ps.Get(), nullptr, 0);
	}

	// Output Merger
	{
		std::array<float, 4> blend_factor = {
			0, 0, 0, 0
		};
		im_ctx3->OMSetBlendState(instance->blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

		std::array<ID3D11RenderTargetView*, 1> srv = {
			window->present_rtv.Get()
		};
		im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);
	}

	// Draw
	im_ctx3->DrawIndexedInstanced(indexes.size(), 1,
		0, 0, 0);
}
