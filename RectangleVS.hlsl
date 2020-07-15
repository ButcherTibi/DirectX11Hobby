
struct VertexIn {
	float2 pos: POSITION;
	uint idx: IDX;
};

struct CommonStuff
{
    uint width;
    float3 pad;
    uint height;
    float3 pad2;
};

StructuredBuffer<CommonStuff> common_stuff : register(t0);

struct VertexOut {
    nointerpolation uint idx : IDX;
    nointerpolation float4 pos : SV_POSITION;
};

VertexOut main(VertexIn input)
{	
    CommonStuff common = common_stuff.Load(0);
    
    VertexOut output;
    output.pos.x = (input.pos.x / common.width) * 2 - 1;
    output.pos.y = (input.pos.y / common.height) * 2 - 1;
    output.pos.y *= -1;
    output.pos.z = 0;
    output.pos.w = 1;
    
	output.idx = input.idx;
	return output;
}