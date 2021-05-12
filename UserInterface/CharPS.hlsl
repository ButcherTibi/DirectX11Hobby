struct PixelInput
{
    float4 pixel_pos : SV_POSITION;
    float2 uv : TEXCOORD;
    
    uint instance_id : INSTANCE_ID;
};

Texture2D<float> atlas;
SamplerState atlas_sampler;

struct Instance {
    float4 color;
};
StructuredBuffer<Instance> sbuff;

float4 main(PixelInput input) : SV_TARGET
{
    
    Instance inst = sbuff.Load(input.instance_id);
    
    float char_pix = atlas.Sample(atlas_sampler, input.uv);
    float4 color = inst.color;
    color.a *= char_pix;
      
    return color;
}