#pragma once

#include "NuiLibrary.hpp"


// must release this before UI
class MeshRenderer {
public:
	float viewport_width;
	float viewport_height;

	std::vector<char> shader_cso;

	// Update
	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* im_ctx3;

	// for overall proper rendering
	dx11::Texture scene_dtex;

	/* created by the solid mesh shader to be compared by the wireframe shader to restrict wireframe only to specific drawcall */ 
	dx11::Texture mesh_mask_dtex;

	/* the see thru wireframe shader must discard pixels only relative to itself */
	dx11::Texture wireframe_dtex;

	//dx11::ConstantBuffer frame_ubuff;
	dx11::Buffer frame_ubuff;

	ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;
	ComPtr<ID3D11VertexShader> octree_vs;

	// Rasterizer State
	dx11::RasterizerState mesh_rs;
	dx11::RasterizerState mesh_none_rs;
	dx11::RasterizerState wire_bias_rs;
	dx11::RasterizerState wire_none_bias_rs;
	dx11::RasterizerState wire_none_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;
	ComPtr<ID3D11PixelShader> mesh_output_depth_ps;
	ComPtr<ID3D11PixelShader> mesh_depth_only_ps;
	ComPtr<ID3D11PixelShader> front_wire_ps;
	ComPtr<ID3D11PixelShader> front_wire_tess_ps;
	ComPtr<ID3D11PixelShader> see_thru_wire_ps;
	ComPtr<ID3D11PixelShader> see_thru_wire_tess_ps;
	ComPtr<ID3D11PixelShader> wire_ps;
	ComPtr<ID3D11PixelShader> wire_tess_ps;

	ComPtr<ID3D11DepthStencilState> depth_stencil;

	ComPtr<ID3D11BlendState> blendless_bs;
	ComPtr<ID3D11BlendState> blend_target_0_bs;

	uint32_t render_target_width;
	uint32_t render_target_height;

public:
	// Internal
	void loadVertices();
	void loadUniform();

public:
	void setWireframeDepthBias(int depth_bias);

	void draw(nui::SurfaceEvent& event);
};

void geometryDraw(nui::Window* window, nui::StoredElement* source, nui::SurfaceEvent& event, void* user_data);
