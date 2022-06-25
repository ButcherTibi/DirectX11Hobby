#pragma once

#include <renderdoc_app.h>
#include <CommonTypes.hpp>
#include "DX11Wrapper.hpp"
#include <SculptMesh/SculptMesh.hpp>


void checkDX11(HRESULT result);

struct Adapter {
	uint32_t index;

	ComPtr<IDXGIAdapter> adapter;
	DXGI_ADAPTER_DESC desc;
};


class MeshRenderer {
public:
	std::vector<char> shader_cso;

	bool load_uniform;

public:
	// Debug
	RENDERDOC_API_1_4_1* render_doc = nullptr;

	// Context
	ComPtr<IDXGIFactory2> factory_2;
	ComPtr<IDXGIFactory6> factory;
	std::vector<Adapter> adapters;
	ComPtr<ID3D11Device> dev1;
	ComPtr<ID3D11Device5> dev;
	ComPtr<ID3D11DeviceContext> im_ctx1;
	ComPtr<ID3D11DeviceContext3> im_ctx;

	// Swapchain
	ComPtr<IDXGISwapChain1> swapchain1;
	std::array<ComPtr<ID3D11Texture2D>, 2> swapchain_texs;
	std::array<ComPtr<ID3D11RenderTargetView>, 2> swapchain_rtvs;
	u32 swapchain_backbuffer_index;
	ID3D11RenderTargetView* present_rtv;


	dx11::StagingBuffer staging_buff;

	// for overall proper rendering
	dx11::Texture scene_dtex;

	/* created by the solid mesh shader to be compared by the wireframe shader to restrict wireframe only to specific drawcall */ 
	dx11::Texture mesh_mask_dtex;

	/* the see thru wireframe shader must discard pixels only relative to itself */
	dx11::Texture wireframe_dtex;

	// World Position of Pixels (RGBA format)
	dx11::Texture world_pos_tex;
	dx11::Texture world_pos_cputex;

	//dx11::ConstantBuffer frame_ubuff;
	dx11::Buffer frame_ubuff;

	//ComPtr<ID3D11InputLayout> mesh_il;

	ComPtr<ID3D11VertexShader> mesh_vs;
	ComPtr<ID3D11VertexShader> octree_vs;

	// Geometry Shader
	ComPtr<ID3D11GeometryShader> mesh_gs;

	// Rasterizer State
	dx11::RasterizerState mesh_rs;
	dx11::RasterizerState mesh_none_rs;
	dx11::RasterizerState wire_bias_rs;
	dx11::RasterizerState wire_none_bias_rs;

	ComPtr<ID3D11PixelShader> mesh_ps;
	ComPtr<ID3D11PixelShader> wire_ps;
	ComPtr<ID3D11PixelShader> mesh_depth_only_ps;
	ComPtr<ID3D11PixelShader> see_thru_wire_ps;
	ComPtr<ID3D11PixelShader> aabb_ps;
	ComPtr<ID3D11PixelShader> debug_ps;

	ComPtr<ID3D11DepthStencilState> depth_stencil;

	ComPtr<ID3D11BlendState> blendless_bs;
	ComPtr<ID3D11BlendState> blend_target_0_bs;

	// Compute Shaders with common/temp buffer data
	ComPtr<ID3D11ComputeShader> distribute_AABB_verts_cs;
	ComPtr<ID3D11ComputeShader> update_vertex_positions_cs;
	ComPtr<ID3D11ComputeShader> update_vertex_normals_cs;
	ComPtr<ID3D11ComputeShader> update_tesselation_triangles;

	dx11::ConstantBuffer mesh_aabb_graph;

	std::vector<GPU_UnplacedVertexGroup> unplaced_verts;
	dx11::ArrayBuffer<GPU_UnplacedVertexGroup> gpu_unplaced_verts;

	std::vector<GPU_PlacedVertexGroup> placed_verts;
	dx11::ArrayBuffer<GPU_PlacedVertexGroup> gpu_placed_verts;

	std::vector<GPU_VertexPositionUpdateGroup> vert_pos_updates;
	dx11::ArrayBuffer<GPU_VertexPositionUpdateGroup> gpu_vert_pos_updates;

	std::vector<GPU_VertexNormalUpdateGroup> vert_normal_updates;
	dx11::ArrayBuffer<GPU_VertexNormalUpdateGroup> gpu_vert_normal_updates;

	std::vector<GPU_PolyNormalUpdateGroup> poly_normal_updates;
	dx11::ArrayBuffer<GPU_PolyNormalUpdateGroup> gpu_poly_normal_updates;
	dx11::ArrayBuffer<GPU_Result_PolyNormalUpdateGroup> gpu_r_poly_normal_updates;
	std::vector<GPU_Result_PolyNormalUpdateGroup> poly_r_normal_updates;

	std::vector<GPU_MeshTriangle> debug_triangles;

	//dx11::ComputeCall<> poly_adds_removes_compute;

	uint32_t render_target_width;
	uint32_t render_target_height;

public:
	// Internal
	void loadVertices();
	void loadUniform();

public:
	void init(bool enable_render_doc);

	// used to shift the wireframe closer to the camera in order not have it be obscured by the solid mesh
	void setWireframeDepthBias(int32_t depth_bias);

	// performs a readback of one pixel for `world_pos_tex` texture
	// if pixel is unused then r_world_pos.x == FLT_MAX
	void getPixelWorldPosition(int32_t x, int32_t y, glm::vec3& r_world_pos);

	void render();
};

extern MeshRenderer renderer;
