struct PixelInput
{
    float4 pixel_pos : SV_POSITION;
    float2 uv : TEXCOORD;
    float4 color : COLOR;
    uint parent_clip_id : PARENT_CLIP_ID;
};

Texture2D<uint> parent_clip_mask;
Texture2D<float> atlas;

SamplerState atlas_sampler;

float4 main(PixelInput input) : SV_TARGET
{
    if (input.parent_clip_id == parent_clip_mask.Load(input.pixel_pos.xyz))
    {  
        float tex = atlas.Sample(atlas_sampler, input.uv);
        float4 color = input.color;
        color.a *= tex;
        
        return color;
    }
    
    discard;
    return float4(0, 0, 0, 0);
}