
// Header
#include "NuiLibrary.hpp"


using namespace nui;


void Window::_render()
{
	// don't render if display too small or window is not visible
	if (win_messages.is_minimized ||
		surface_width < 10 || surface_height < 10)
	{
		return;
	}

	auto& im_ctx3 = instance->im_ctx3;

	// Load Character Atlas
	instance->loadCharacterAtlasToTexture();

	// Load Character Vertices
	if (char_verts.size()) {
		char_vbuff.load(char_verts.data(), sizeof(GPU_CharacterVertex) * char_verts.size());
	}

	// Load Character Indexes
	if (char_idxs.size()) {
		char_idxbuff.load(char_idxs.data(), sizeof(uint32_t) * char_idxs.size());
	}

	// Load Text Instances
	if (text_instances.size()) {
		text_instabuff.load(text_instances.data(), sizeof(GPU_TextInstance) * text_instances.size());
	}

	// Rect Character Vertices
	rect_vbuff.load(rect_verts.data(), sizeof(GPU_RectVertex) * rect_verts.size());

	// Rect Character Indexes
	rect_idxbuff.load(rect_idxs.data(), sizeof(uint32_t) * rect_idxs.size());

	// Load Constant Buffer
	cbuff.load(&gpu_constants, sizeof(GPU_Constants));

	im_ctx3->ClearState();

	// Resize Attachments
	{
		DXGI_SWAP_CHAIN_DESC desc;
		swapchain1->GetDesc(&desc);

		if (desc.BufferDesc.Width != surface_width || desc.BufferDesc.Height != surface_height) {

			present_rtv->Release();
			present_img->Release();

			throwDX11(swapchain1->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, desc.Flags));

			throwDX11(swapchain1->GetBuffer(0, IID_PPV_ARGS(present_img.GetAddressOf())));

			ID3D11Device5* dev5 = instance->dev5.Get();
			throwDX11(dev5->CreateRenderTargetView(present_img.Get(), NULL, present_rtv.GetAddressOf()));
		}
	}

	std::array<float, 4> clear_color = { 0, 0, 0, 0 };
	im_ctx3->ClearRenderTargetView(present_rtv.Get(), clear_color.data());

	for (auto& render_stack : render_stacks) {
		
		for (StoredElement* stored_elem : render_stack.second) {

			switch (stored_elem->index()) {
			case ElementType::ROOT: {
				break;
			}
			case ElementType::TEXT: {

				auto text = std::get_if<Text>(stored_elem);

				if (!text->text.size()) {
					continue;
				}

				// Input Assembly
				im_ctx3->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				im_ctx3->IASetInputLayout(instance->char_input_layout.Get());

				std::array<ID3D11Buffer*, 2> vertex_buffs = {
					char_vbuff.get(), text_instabuff.get()
				};
				std::array<uint32_t, 2> strides = {
					sizeof(GPU_CharacterVertex), sizeof(GPU_TextInstance)
				};
				std::array<uint32_t, 2> offsets = {
					0, 0
				};

				im_ctx3->IASetVertexBuffers(0, vertex_buffs.size(), vertex_buffs.data(), strides.data(), offsets.data());
				im_ctx3->IASetIndexBuffer(char_idxbuff.get(), DXGI_FORMAT_R32_UINT, 0);

				// Vertex Shader
				std::array<ID3D11Buffer*, 1> vs_cbuffs = {
					cbuff.get()
				};

				im_ctx3->VSSetConstantBuffers(0, vs_cbuffs.size(), vs_cbuffs.data());

				im_ctx3->VSSetShader(instance->char_vs.Get(), nullptr, 0);

				// Rasterizer
				im_ctx3->RSSetViewports(1, &viewport);

				im_ctx3->RSSetState(instance->solid_back_rs.Get());

				// Pixel Shader
				std::array<ID3D11SamplerState*, 1> samplers = {
					instance->char_atlas_sampler.Get()
				};

				im_ctx3->PSSetSamplers(0, samplers.size(), samplers.data());

				std::array<ID3D11ShaderResourceView*, 1> ps_srv = {
					instance->char_atlas_tex.getSRV()
				};

				im_ctx3->PSSetShaderResources(0, ps_srv.size(), ps_srv.data());

				im_ctx3->PSSetShader(instance->char_ps.Get(), nullptr, 0);

				// Output Merger
				std::array<float, 4> blend_factor = {
					0, 0, 0, 0
				};

				im_ctx3->OMSetBlendState(instance->blend_state.Get(), blend_factor.data(), 0xFFFF'FFFF);

				std::array<ID3D11RenderTargetView*, 1> srv = {
					present_rtv.Get()
				};

				im_ctx3->OMSetRenderTargets(srv.size(), srv.data(), nullptr);

				// Draw
				im_ctx3->DrawIndexedInstanced(text->_index_count, 1,
					text->_index_start_idx, text->_vertex_start_idx, text->_instance_start_idx);

				break;
			}

			case ElementType::RELATIVE_WRAP: {

				auto rel = std::get_if<RelativeWrap>(stored_elem);
				rel->_draw();
				break;
			}

			case ElementType::GRID: {
				
				auto grid = std::get_if<Grid>(stored_elem);
				grid->_draw();
				break;
			}
			}
		}
	}

	throwDX11(swapchain1->Present(0, 0));
}
