
struct VertexInput
{
    int2 pos : POSITION;
    float2 uv : TEXCOORD;
    
    float4 color : COLOR;
};

cbuffer Commons : register(b0)
{
    int2 screen_size;
};

struct VertexOutput
{
    float4 dx11_pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    
    float2 debug_pos : DBG0;
    float2 debug_screen_size : DBG1;
};

VertexOutput main(VertexInput input)
{  
    float2 local_pos;
    local_pos.x = input.pos.x / (float)screen_size.x;
    local_pos.y = input.pos.y / (float)screen_size.y;

    float4 dx11_pos = float4(
        local_pos.x * 2 - 1,
        -(local_pos.y * 2 - 1),
        0,
        1
    );
    
    VertexOutput output;
    output.dx11_pos = dx11_pos;
    output.uv = input.uv;
    output.color = input.color;
    
    output.debug_pos = (float2)input.pos;
    output.debug_screen_size = (float2)screen_size;
    
    return output;
}