#include "MeshPixelIn.hlsli"


struct PixelOutput {
	float4 color : SV_Target0;
	float depth_mask : SV_Target1;
	uint instance_id : SV_Target2;
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
};

PixelOutput main(PixelIn input)
{
	float3 pbr_color = calcPhysicalBasedRendering(
		input.normal, -camera_forward, lights,
		input.roughness, input.metallic, input.specular,
		input.albedo_color, ambient_intensity);
	
	PixelOutput output;
	output.color = float4(pbr_color, 1.);
	output.depth_mask = input.dx_pos.z;
	output.instance_id = input.instance_id;
	
	return output;
}