#pragma once

#include "NuiLibrary.hpp"
// #include "Application.hpp"
#include "GPU_ShaderTypesMesh.hpp"


// must release this before UI
class MeshRenderer {
public:
	float viewport_width;
	float viewport_height;

	std::vector<char> mesh_vs_cso;
	std::vector<char> mesh_ps_cso;
	std::vector<char> mesh_output_depth_ps_cso;
	std::vector<char> dim_wireframe_ps_cso;
	std::vector<char> wireframe_with_tessellation_ps_cso;

	// Update
	bool load_vertices;
	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* im_ctx3;
	ID3D11DeviceContext3* de_ctx3;

	// Masks
	// ComPtr<ID3D11Texture2D> instance_id_tex;
	// ComPtr<ID3D11ShaderResourceView> instance_id_srv;
	// ComPtr<ID3D11RenderTargetView> instance_id_rtv;

	// Depth Textures

	// for overall proper rendering
	ComPtr<ID3D11Texture2D> scene_dtex;
	ComPtr<ID3D11DepthStencilView> scene_dsv;
	// ComPtr<ID3D11ShaderResourceView> scene_depth_srv;

	/* created by the solid mesh shader to be compared by the wireframe shader to restrict wireframe only to specific drawcall */ 
	ComPtr<ID3D11Texture2D> mesh_mask_dtex;
	ComPtr<ID3D11RenderTargetView> mesh_mask_rtv;
	ComPtr<ID3D11ShaderResourceView> mesh_mask_srv;

	/* the see thru wireframe shader must discard pixels only relative to itself */
	ComPtr<ID3D11Texture2D> wireframe_dtex;
	ComPtr<ID3D11DepthStencilView> wireframe_dsv;

	dx11::Buffer vbuff;
	dx11::Buffer instabuff;
	dx11::Buffer frame_ubuff;
	dx11::Buffer drawcall_ubuff;

	dx11::Buffer aabbs_vbuff;
	dx11::Buffer aabbs_ibuff;

	ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;

	ComPtr<ID3D11RasterizerState> fill_front_rs;
	ComPtr<ID3D11RasterizerState> fill_none_rs;
	ComPtr<ID3D11RasterizerState> wireframe_bias_rs;
	ComPtr<ID3D11RasterizerState> wireframe_none_bias_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;
	ComPtr<ID3D11PixelShader> mesh_output_depth_ps;
	ComPtr<ID3D11PixelShader> dim_wireframe_ps;
	ComPtr<ID3D11PixelShader> wireframe_with_tesselation_ps;

	ComPtr<ID3D11DepthStencilState> greater_dss;

	ComPtr<ID3D11BlendState> mesh_bs;
	ComPtr<ID3D11BlendState> blend_target_0_bs;

	uint32_t render_target_width;
	uint32_t render_target_height;

public:
	ErrStack loadVertices();
	ErrStack loadUniform();

	ErrStack draw(nui::SurfaceEvent& event);
};

ErrStack geometryDraw(nui::SurfaceEvent& event);
