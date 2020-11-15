#pragma once

#include "NuiLibrary.hpp"
// #include "Application.hpp"
#include "GPU_ShaderTypesMesh.hpp"


/* TODO:
- camera quaternion
- camera controls
- import GLTF
- vertex shading
- triangle shading
- separate CPU and GPU dependend operations
- no clipping of viewport required

- multiple objects
- menu entry File
  - New File
  - Import GLTF
- menu entry Settings
  - Controls
  - Viewport
*/


// must release this before UI
class MeshRenderer {
public:
	//uint32_t render_width;
	//uint32_t render_height;

	uint32_t viewport_width;
	uint32_t viewport_height;

	std::vector<GPU_MeshVertex> vertices;
	std::vector<GPU_MeshInstance> instances;
	GPU_MeshUniform uniform;

	std::vector<char> mesh_vs_cso;
	std::vector<char> mesh_ps_cso;

	// Update
	bool load_vertices;
	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* de_ctx3;

	dx11::Buffer vbuff;
	dx11::Buffer instabuff;
	dx11::Buffer ubuff;

	ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;

	ComPtr<ID3D11RasterizerState> mesh_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;

	ComPtr<ID3D11BlendState> mesh_bs;

public:
	void generateVertices();
	void generateUniform();

	ErrStack loadVertices();
	ErrStack loadUniform();
	// nui::ErrStack resizeAtachments();

	ErrStack draw(nui::SurfaceEvent& event);
};

ErrStack geometryDraw(nui::SurfaceEvent& event);
