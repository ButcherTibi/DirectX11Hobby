struct VertexInput
{
	float4 dx_pos : SV_POSITION;
	float3 normal : NORMAL;
};

float4 main(VertexInput input) : SV_TARGET
{
	//return float4(abs(input.normal.x), abs(input.normal.y), abs(input.normal.z), 1.0f);
	return float4(1, 1, 1, 1);
}