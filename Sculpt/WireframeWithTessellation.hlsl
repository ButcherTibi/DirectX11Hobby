struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
	float tess_edge : TESSELLATION_EDGE;
	float tess_edge_dir : TESSELLATION_EDGE_DIR;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
	float3 wireframe_tess_front_color : WIREFRAME_TESS_FRONT_COLOR;
	float4 wireframe_tess_back_color : WIREFRAME_TESS_BACK_COLOR;
	float wireframe_tess_split_count : WIREFRAME_TESS_SPLIT_COUNT;
	float wireframe_tess_gap : WIREFRAME_TESS_GAP;
};

struct CameraLight {
	float3 normal;
	float3 color;
	float intensity;
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

cbuffer DrawcallUniforms : register(b1)
{
	float flip_surface_normal;
}

Texture2D<float> mesh_depth_tex;


float4 main(VertexInput input) : SV_TARGET
{
	float mesh_depth = mesh_depth_tex.Load(input.dx_pos.xyz);
	float wireframe_depth = input.dx_pos.z;
	
	// back
	if (wireframe_depth >= mesh_depth) {
		
		// tesselation edge
		if (input.tess_edge >= 1) {
			float stripe = floor(frac(input.tess_edge_dir * input.wireframe_tess_split_count) + input.wireframe_tess_gap);
		
			if (stripe > 0.5) {
				return float4(input.wireframe_tess_back_color);
			}
		
			discard;
			return float4(9, 9, 9, 9);
		}
		
		return float4(input.wireframe_back_color);
	}
	
	// front
	if (input.tess_edge >= 1) {
		float stripe = floor(frac(input.tess_edge_dir * input.wireframe_tess_split_count) + input.wireframe_tess_gap);
		
		if (stripe > 0.5) {
			return float4(input.wireframe_tess_front_color.rgb, 1);
		}
		
		discard;
		return float4(9, 9, 9, 9);
	}
	
	return float4(input.wireframe_front_color, 1);
}