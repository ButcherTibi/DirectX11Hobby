#include "MeshPixelIn.hlsli"

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

Texture2D<float> mesh_depth_tex;
StructuredBuffer<Instance> instances;

[earlydepthstencil]
float4 main(PixelIn input) : SV_TARGET0
{
	float mesh_mask_depth = mesh_depth_tex.Load(input.pos.xyz);
	float depth = input.pos.z;
	
	if (mesh_mask_depth < 1.) {
		
		Instance instance = instances.Load(input.instance_id);
		
		switch (shading_normal) {
		// VERTEX | POLY
		case 0:
		case 1: {
			if (input.tess_edge != 1.) {					
				if (depth > mesh_mask_depth) {
					return float4(instance.wireframe_back_color);
				}
				return float4(instance.wireframe_front_color, 1);
			}
			break;
		}
		// TESSELATION
		default: {
			if (input.tess_edge == 1) {
				
				float stripe = calcStripe(
					input.tess_edge_dir, instance.wireframe_tess_split_count, instance.wireframe_tess_gap);
		
				if (stripe > 0.5) {
					if (depth > mesh_mask_depth) {
						return float4(instance.wireframe_tess_back_color);
					}
					else {
						return float4(instance.wireframe_tess_front_color.rgb, 1);
					}
				}
			}
			else {
				if (depth > mesh_mask_depth) {
					return float4(instance.wireframe_tess_back_color);
				}
				else {
					return float4(instance.wireframe_tess_front_color.rgb, 1);
				}
			}
			break;
		}
		}
	}
	
	//if (input.tess_edge != 1) {
	//	if (depth < mesh_mask_depth) {
	//		return float4(1, 0, 0, 1);
	//	}
	//	else if (depth > mesh_mask_depth) {
	//		return float4(0, 1, 0, 1);
	//	}
	//	return float4(0, 0, 1, 1);
	//}
	//else if (mesh_mask_depth < 1.) {
	//	return float4(1, 1, 1, 1);
	//}
	
	discard;
	return float4(1, 0, 1, 1);
}