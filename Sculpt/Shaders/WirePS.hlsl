#include "MeshPixelIn.hlsli"


StructuredBuffer<Instance> instances;

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

[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET0
{
	Instance instance = instances.Load(input.instance_id);
	
	switch (shading_normal) {
	// VERTEX | POLY
	case 0:
	case 1: {
		if (input.tess_edge != 1) {
			return float4(instance.wireframe_front_color, 1);
		}
		break;
	}
	// TESSELATION
	default: {
		if (input.tess_edge >= 1) {
		
			float stripe = calcStripe(input.tess_edge_dir,
				instance.wireframe_tess_split_count, instance.wireframe_tess_gap);
		
			if (stripe > 0.5) {
				return float4(instance.wireframe_tess_front_color, 1);
			}
		
			discard;
			return float4(1, 0, 1, 1);
		}
	
		return float4(instance.wireframe_front_color, 1);
		break;
	}
	}
	
	discard;
	return float4(1, 0, 1, 1);
}