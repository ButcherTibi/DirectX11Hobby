
struct PixelIn
{
    uint idx : IDX;
};

struct Props
{
    float4 color;
    float4 center_radius;
};

StructuredBuffer<Props> props : register(t0);

float4 main(PixelIn input) : SV_TARGET
{
    Props elem_prop = props.Load(input.idx);
    float2 center = elem_prop.center_radius.xy;
    float radius = elem_prop.center_radius.z;
    
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
}