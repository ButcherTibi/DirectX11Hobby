
struct VertexInput
{
    float2 pos : POSITION;
    float2 uv : TEXCOORD;
    
    // Instance
    float4 color : COLOR;
    float2 inst_pos : INSTANCE_POSITION;
    float raster_size : RASTERIZED_SIZE;
    float size : SIZE;
    uint parent_clip_id : PARENT_CLIP_ID;
};

cbuffer Commons : register(b0)
{
    float4 screen_size;
};

struct VertexOutput
{
    float4 dx11_pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint parent_clip_id : PARENT_CLIP_ID;
};

VertexOutput main(VertexInput input)
{
    float screen_width = screen_size.x;
    float screen_height = screen_size.y;
    
    float2 local_pos = input.pos;
    local_pos *= input.size / input.raster_size;
    local_pos += input.inst_pos;

    float4 dx11_pos = float4(
        local_pos.x / screen_width * 2 - 1,
        local_pos.y / screen_height * 2 - 1,
        0,
        1
    );
    dx11_pos.y *= -1;
    
    VertexOutput output;
    output.dx11_pos = dx11_pos;
    output.uv = input.uv;
    output.color = input.color;
    output.parent_clip_id = input.parent_clip_id;
    return output;
}