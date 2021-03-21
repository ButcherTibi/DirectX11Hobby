struct PixelInput
{
    float4 pixel_pos : SV_POSITION;
    float2 uv : TEXCOORD;
    
    float4 color : COLOR;
};

Texture2D<float> atlas;
SamplerState atlas_sampler;

float4 main(PixelInput input) : SV_TARGET
{
    float char_pix = atlas.Sample(atlas_sampler, input.uv);
    float4 color = input.color;
    color.a *= char_pix;
      
    return color;
    //return float4(1, 0, 0, 1);
}