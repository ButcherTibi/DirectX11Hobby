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
DirectX::XMFLOAT4 dxConvert(glm::vec4& value);
DirectX::XMFLOAT4 dxConvert(glm::quat& value);
DirectX::XMFLOAT4X4 dxConvert(glm::mat4& value);
DirectX::XMMATRIX dxConvertMatrix(glm::mat4& value);

struct GPU_MeshVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	float tess_edge;
	float tess_edge_dir;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 4> getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 4> elems;
		elems[0].SemanticName = "POSITION";
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[1].SemanticName = "NORMAL";
		elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[2].SemanticName = "TESSELLATION_EDGE";
		elems[2].Format = DXGI_FORMAT_R32_FLOAT;

		elems[3].SemanticName = "TESSELLATION_EDGE_DIR";
		elems[3].Format = DXGI_FORMAT_R32_FLOAT;

		for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
			elem.SemanticIndex = 0;
			elem.InputSlot = 0;
			elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elem.InstanceDataStepRate = 0;
		}

		return elems;
	}
};


struct GPU_AABB_Vertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 color;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 2> getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 2> elems;
		elems[0].SemanticName = "POSITION";
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[1].SemanticName = "COLOR";
		elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
			elem.SemanticIndex = 0;
			elem.InputSlot = 0;
			elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elem.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			elem.InstanceDataStepRate = 0;
		}

		return elems;
	}
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

	static std::array<D3D11_INPUT_ELEMENT_DESC, 12> getInputLayout(uint32_t input_slot = 1)
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 12> elems;
		elems[0].SemanticName = "INSTANCE_POSITION";
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[1].SemanticName = "INSTANCE_ROTATION";
		elems[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		elems[2].SemanticName = "ALBEDO_COLOR";
		elems[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[3].SemanticName = "ROUGHNESS";
		elems[3].Format = DXGI_FORMAT_R32_FLOAT;

		elems[4].SemanticName = "METALLIC";
		elems[4].Format = DXGI_FORMAT_R32_FLOAT;

		elems[5].SemanticName = "SPECULAR";
		elems[5].Format = DXGI_FORMAT_R32_FLOAT;

		elems[6].SemanticName = "WIREFRAME_FRONT_COLOR";
		elems[6].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[7].SemanticName = "WIREFRAME_BACK_COLOR";
		elems[7].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		elems[8].SemanticName = "WIREFRAME_TESS_FRONT_COLOR";
		elems[8].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[9].SemanticName = "WIREFRAME_TESS_BACK_COLOR";
		elems[9].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		elems[10].SemanticName = "WIREFRAME_TESS_SPLIT_COUNT";
		elems[10].Format = DXGI_FORMAT_R32_FLOAT;

		elems[11].SemanticName = "WIREFRAME_TESS_GAP";
		elems[11].Format = DXGI_FORMAT_R32_FLOAT;

		for (D3D11_INPUT_ELEMENT_DESC& elem : elems) {
			elem.SemanticIndex = 0;
			elem.InputSlot = input_slot;
			elem.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
			elem.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
			elem.InstanceDataStepRate = 1;
		}

		return elems;
	}
};


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
	uint32_t _pad_3[3];
};

struct GPU_DrawcallUniform {
	uint32_t instance_id;
	uint32_t _pad_0[3];
	//--------------------------------
};

#pragma pack()
