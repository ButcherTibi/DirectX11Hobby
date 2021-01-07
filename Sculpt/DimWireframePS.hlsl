struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
	
	float3 albedo_color : ALBEDO_COLOR;
	float roughness : ROUGHNESS;
	float metallic : METALLIC;
	float specular : SPECULAR;
	
	float3 wireframe_front_color : WIREFRAME_FRONT_COLOR;
	float4 wireframe_back_color : WIREFRAME_BACK_COLOR;
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
		return float4(input.wireframe_back_color);
	}
	
	// front
	return float4(input.wireframe_front_color, 1.);
}