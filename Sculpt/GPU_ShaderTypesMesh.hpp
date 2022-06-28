#pragma once

// Standard
#include <array>

// GLM
#include <glm\vec3.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\mat4x4.hpp>

// DirectX 11
#include <DirectXMath.h>
#include <d3d11_4.h>


DirectX::XMFLOAT3 dxConvert(glm::vec3& value);
DirectX::XMFLOAT3 dxConvert(float x, float y, float z);
DirectX::XMFLOAT4 dxConvert(glm::vec4& value);
DirectX::XMFLOAT4 dxConvert(glm::quat& value);
DirectX::XMFLOAT4X4 dxConvert(glm::mat4& value);
DirectX::XMMATRIX dxConvertMatrix(glm::mat4& value);

glm::vec3 glmConvert(DirectX::XMFLOAT3& value);


struct GPU_MeshVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;

	/*static auto getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> nodes;
		nodes[0].SemanticName = "POSITION";
		nodes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		nodes[1].SemanticName = "NORMAL";
		nodes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		for (D3D11_INPUT_ELEMENT_DESC& elem : nodes) {
			elem.SemanticIndex = 0;
			elem.InputSlot = 0;
			elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elem.InstanceDataStepRate = 0;
		}

		return nodes;
	}*/
};


struct GPU_MeshTriangle {
	DirectX::XMFLOAT3 poly_normal;
	DirectX::XMFLOAT3 tess_normal;
	uint32_t tess_vertex_0;
	uint32_t tess_vertex_1;
};


struct GPU_MeshInstance {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 rot;

	DirectX::XMFLOAT3 albedo_color;
	float roughness;
	float metallic;
	float specular;

	DirectX::XMFLOAT3 wireframe_front_color;
	DirectX::XMFLOAT4 wireframe_back_color;
	DirectX::XMFLOAT3 wireframe_tess_front_color;
	DirectX::XMFLOAT4 wireframe_tess_back_color;
	float wireframe_tess_split_count;
	float wireframe_tess_gap;
};

struct GPU_BrushCircleInstance {
	DirectX::XMFLOAT3 pos;  // position of the brush
	DirectX::XMFLOAT3 normal;  // surface normal to angle the brush
	float radius;
	DirectX::XMFLOAT4 color;
};


struct GPU_UnplacedVertexGroup {
	uint32_t vert_idxs[21];
};

struct GPU_PlacedVertexGroup {
	// vertex ,axis, level
	int32_t vert_idxs[21][3][5];
};

struct GPU_VertexPositionUpdateGroup {
	uint32_t vertex_id[64];
	DirectX::XMFLOAT3 new_pos[64];
};

struct GPU_VertexNormalUpdateGroup {
	uint32_t vertex_id[64];
	DirectX::XMFLOAT3 new_normal[64];
};

// each poly is made of TWO tesselation triangles
struct GPU_PolyNormalUpdateGroup {
	uint32_t tess_idxs[32][2];  // idx of tess triangles to update
	uint32_t poly_verts[32][4];  // vertices of polygon
	uint32_t tess_type[32];  // how is the polygon split
	uint32_t tess_split_vertices[32][2];
};

struct GPU_Result_PolyNormalUpdateGroup {
	glm::vec3 poly_normal[32];
	glm::vec3 tess_normals[32][2];
};

//struct GPU_PolyListUpdateGroup {
//	uint32_t tess_idxs[32][2];
//	uint32_t tess_vertex_0[32];
//	uint32_t tess_vertex_1[32];
//};


namespace GPU_AABB_Graph_Fields {
	enum {
		ROOT_SIZE,
		LEVELS
	};
}


/* WARNING: All structs below are HLSL binary compatible with shader constant buffer */
#pragma pack(16)

struct GPU_CameraLight {
	DirectX::XMFLOAT3 normal;
	uint32_t pad_0;
	//--------------------------------
	DirectX::XMFLOAT3 color;
	float intensity;
	//--------------------------------
};

struct GPU_MeshUniform {
	DirectX::XMFLOAT3 camera_pos;
	uint32_t _pad_0;
	//--------------------------------
	DirectX::XMFLOAT4 camera_quat;
	//--------------------------------
	DirectX::XMFLOAT3 camera_forward;
	uint32_t _pad_1;
	//--------------------------------
	DirectX::XMMATRIX perspective_matrix;
	//--------------------------------
	float z_near;
	float z_far;
	uint32_t _pad_2[2];
	//--------------------------------
	GPU_CameraLight lights[8];
	//--------------------------------
	float ambient_intensity;
	uint32_t shading_normal;
	uint32_t _pad_3[2];
	//--------------------------------
};

struct GPU_DrawcallUniform {
	uint32_t instance_id;
	uint32_t _pad_0[3];
	//--------------------------------
};

#pragma pack()
