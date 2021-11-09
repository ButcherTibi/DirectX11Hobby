#pragma once

#include "DX11Wrapper.hpp"

import UserInterface;

#include "SculptMesh.hpp"


// must release this before UI
class MeshRenderer {
public:
	float viewport_width;
	float viewport_height;

	std::vector<char> shader_cso;

	bool load_uniform;

public:
	ID3D11Device5* dev5 = nullptr;
	ID3D11DeviceContext3* im_ctx3;

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

	uint32_t render_target_width;
	uint32_t render_target_height;

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


public:
	// Internal
	void loadVertices();
	void loadUniform();

public:
	// used to shift the wireframe closer to the camera in order not have it be obscured by the solid mesh
	void setWireframeDepthBias(int32_t depth_bias);

	// performs a readback of one pixel for `world_pos_tex` texture
	// if pixel is unused then r_world_pos.x == FLT_MAX
	void getPixelWorldPosition(int32_t x, int32_t y, glm::vec3& r_world_pos);
	struct CachedPixelWorldPosition {
		int32_t x;
		int32_t y;
		glm::vec3 world_pos;
	};
	std::vector<CachedPixelWorldPosition> _cached_pixel_world_pos;

	void draw(nui::DirectX11_DrawEvent& event);
};

extern MeshRenderer renderer;

void geometryDraw(nui::Window* window, nui::StoredElement2* source, nui::DirectX11_DrawEvent& event, void* user_data);
