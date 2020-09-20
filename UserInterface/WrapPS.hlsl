struct PixelInput
{
    float4 pixel_pos : SV_POSITION;
    float4 color : COLOR;
    uint parent_clip_id : PARENT_CLIP_ID;
    uint child_clip_id : CHILD_CLIP_ID;
};

Texture2D<uint> parent_clip_mask;

struct PixelOutput
{
    float4 color : SV_TARGET;
    uint next_parent_clip_mask : SV_TARGET1;
};

PixelOutput main(PixelInput input)
{ 
    if (input.parent_clip_id == parent_clip_mask.Load(input.pixel_pos.xyz))
    {
        PixelOutput output;
        output.color = input.color;
        output.next_parent_clip_mask = input.child_clip_id;
        
        return output;
    }
    discard;
    
    #pragma warning(disable : 3578)
    PixelOutput output;
    return output;
}