
struct PixelIn {
	uint idx: IDX;
};

struct CircleProps
{
    float4 color;
};

StructuredBuffer<CircleProps> circle_props : register(t0);

float4 main(PixelIn input) : SV_TARGET
{   
    CircleProps elem_prop = circle_props.Load(input.idx);
    return elem_prop.color;
}