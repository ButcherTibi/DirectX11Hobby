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
	std::vector<char> dim_wireframe_ps_cso;

	// Update
	bool load_vertices;
	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* im_ctx3;
	ID3D11DeviceContext3* de_ctx3;

	ComPtr<ID3D11Texture2D> mesh_depth_tex;
	ComPtr<ID3D11Texture2D> wireframe_depth_tex;

	ComPtr<ID3D11DepthStencilView> mesh_depth_view;
	ComPtr<ID3D11DepthStencilView> wireframe_depth_view;
	
	ComPtr<ID3D11ShaderResourceView> mesh_depth_srv;

	dx11::Buffer vbuff;
	dx11::Buffer instabuff;
	dx11::Buffer frame_ubuff;
	dx11::Buffer drawcall_ubuff;

	dx11::Buffer aabbs_vbuff;
	dx11::Buffer aabbs_ibuff;

	ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;

	ComPtr<ID3D11RasterizerState> mesh_rs;
	ComPtr<ID3D11RasterizerState> wireframe_none_bias_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;
	ComPtr<ID3D11PixelShader> dim_wireframe_ps;

	ComPtr<ID3D11DepthStencilState> mesh_dss;
	ComPtr<ID3D11DepthStencilState> dim_wireframe_dss;

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
