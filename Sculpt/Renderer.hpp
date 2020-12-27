#pragma once

#include "NuiLibrary.hpp"
// #include "Application.hpp"
#include "GPU_ShaderTypesMesh.hpp"


struct DrawMesh {
	uint32_t vertex_start;
	uint32_t vertex_count;
	uint32_t instance_start;
};


// must release this before UI
class MeshRenderer {
public:
	float viewport_width;
	float viewport_height;
	std::vector<DrawMesh> mesh_draws;

	std::vector<char> mesh_vs_cso;
	std::vector<char> mesh_ps_cso;

	// Update
	bool load_vertices;
	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext4* im_ctx4;
	ID3D11DeviceContext3* de_ctx3;

	ComPtr<ID3D11Texture2D> depth_tex;

	ComPtr<ID3D11DepthStencilView> depth_view;

	dx11::Buffer vbuff;
	dx11::Buffer instabuff;
	dx11::Buffer ubuff;

	ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;

	ComPtr<ID3D11RasterizerState> mesh_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;

	ComPtr<ID3D11DepthStencilState> mesh_dss;
	ComPtr<ID3D11BlendState> mesh_bs;

	uint32_t render_target_width;
	uint32_t render_target_height;

public:
	ErrStack loadVertices();
	ErrStack loadUniform();

	ErrStack draw(nui::SurfaceEvent& event);
};

ErrStack geometryDraw(nui::SurfaceEvent& event);
