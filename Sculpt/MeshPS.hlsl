#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float4 color : SV_Target0;
	//uint2 instance_poly_id : SV_Target1;
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

struct MeshTriangle {
	float3 normal;
};

StructuredBuffer<MeshTriangle> mesh_triangles;

PixelOutput main(PixelIn input, uint primitive_id : SV_PrimitiveID)
{
	float3 normal;
	
	// VERTEX
	if (shading_normal == 0) {
		normal = input.normal;
	}
	// POLY | TESSELATION
	else {
		normal = mesh_triangles[primitive_id].normal;
	}
	
	float3 pbr_color = calcPhysicalBasedRendering(
		normal, -camera_forward, lights,
		input.roughness, input.metallic, input.specular,
		input.albedo_color, ambient_intensity);
	
	PixelOutput output;
	output.color = float4(pbr_color, 1.);
	//output.instance_poly_id[0] = input.instance_id;
	//output.instance_poly_id[1] = input.poly_id;
	
	return output;
}