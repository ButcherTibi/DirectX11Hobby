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
DirectX::XMFLOAT4 dxConvert(glm::quat& value);
DirectX::XMFLOAT4X4 dxConvert(glm::mat4& value);


struct GPU_MeshVertex {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 vertex_normal;
	DirectX::XMFLOAT3 tess_normal;
	DirectX::XMFLOAT3 poly_normal;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 4> getInputLayout()
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 4> elems;
		elems[0].SemanticName = "POSITION";
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[1].SemanticName = "VERTEX_NORMAL";
		elems[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[2].SemanticName = "TESSELATION_NORMAL";
		elems[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[3].SemanticName = "POLY_NORMAL";
		elems[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;

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

	uint32_t shading_mode;
	DirectX::XMFLOAT3 albedo_color;
	float roughness;
	float metallic;
	float specular;

	static std::array<D3D11_INPUT_ELEMENT_DESC, 7> getInputLayout(uint32_t input_slot = 1)
	{
		std::array<D3D11_INPUT_ELEMENT_DESC, 7> elems;
		elems[0].SemanticName = "INSTANCE_POSITION";
		elems[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[1].SemanticName = "INSTANCE_ROTATION";
		elems[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;

		elems[2].SemanticName = "SHADING_MODE";
		elems[2].Format = DXGI_FORMAT_R32_UINT;

		elems[3].SemanticName = "ALBEDO_COLOR";
		elems[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;

		elems[4].SemanticName = "ROUGHNESS";
		elems[4].Format = DXGI_FORMAT_R32_FLOAT;

		elems[5].SemanticName = "METALLIC";
		elems[5].Format = DXGI_FORMAT_R32_FLOAT;

		elems[6].SemanticName = "SPECULAR";
		elems[6].Format = DXGI_FORMAT_R32_FLOAT;

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

#pragma pack()
