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

float calcStripe(float gradient, float split_count, float gap_size)
{
	return floor(frac(gradient * split_count) + gap_size);
}

[earlydepthstencil]
float4 main(VertexInput input) : SV_TARGET
{	
	if (input.tess_edge >= 1) {
		
		float stripe = calcStripe(input.tess_edge_dir, input.wireframe_tess_split_count, input.wireframe_tess_gap);
		
		if (stripe > 0.5) {
			return float4(input.wireframe_tess_front_color, 1);
		}
		
		discard;
		return float4(1, 0, 1, 1);
	}
	
	return float4(input.wireframe_front_color, 1);
}