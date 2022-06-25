#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float4 color : SV_Target0;
	float4 world_pos : SV_Target1;
};

cbuffer FrameUniforms : register(b0)
{
	float3 camera_pos;
	float4 camera_quat_inv;
	float3 camera_forward;
	matrix perspective;
	float z_near;
	float z_far;
	
	CameraLight lights[8];
	float ambient_intensity;
	uint shading_normal;
};

StructuredBuffer<MeshTriangle> mesh_triangles;

StructuredBuffer<Instance> instances;


[earlydepthstencil]
PixelOutput main(PixelIn input, uint primitive_id : SV_PrimitiveID)
{
	float3 normal;
	
	switch (shading_normal) {
	// VERTEX
	case 0: {
		normal = input.vertex_normal;
		break;
	}
	// POLY
	case 1: {
		normal = mesh_triangles[primitive_id].poly_normal;
		break;
	}
	//	TESSELATION
	default: {
		normal = mesh_triangles[primitive_id].tess_normal;
		break;
	}
	}
	
	Instance instance = instances.Load(input.instance_id);
	
	float3 pbr_color = calcPhysicalBasedRendering(
		normal, -camera_forward, lights,
		instance.roughness, instance.metallic, instance.specular,
		instance.albedo_color, ambient_intensity);
	
	PixelOutput output;
	output.color = float4(pbr_color, 1.);
	output.world_pos = float4(input.world_pos, 0.12345);
	
	return output;
}