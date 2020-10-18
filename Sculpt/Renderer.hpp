#pragma once

// TODO: better not rely on UI DirectX
#include "NuiLibrary.hpp"
#include "Application.hpp"
#include "GPU_ShaderTypesMesh.hpp"


// must release this before UI
class MeshRenderer {
public:
	uint32_t render_width;
	uint32_t render_height;

	std::vector<GPU_MeshVertex> vertices;
	std::vector<GPU_MeshInstance> instances;
	GPU_MeshUniform uniform;

	std::vector<char> mesh_vs_cso;
	std::vector<char> mesh_ps_cso;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* de_ctx3;

	ID3D11RenderTargetView* compose_rtv;

	dx11::Buffer vbuff;
	dx11::Buffer instabuff;
	dx11::Buffer ubuff;

	ComPtr<ID3D11VertexShader> mesh_vs;

	ComPtr<ID3D11PixelShader> mesh_ps;

public:
	ErrStack loadVertices();
	ErrStack loadUniform();
	// nui::ErrStack resizeAtachments();
};

ErrStack geometryDraw(nui::SurfaceEvent& event);
