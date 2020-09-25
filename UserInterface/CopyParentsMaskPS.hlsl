
Texture2D<uint> next_parents_mask;

uint main(float4 pixel_pos : SV_Position) : SV_TARGET
{
    uint next_pix = next_parents_mask.Load(pixel_pos.xyz);
    
    if (next_pix)
    {
        return next_pix;
    }
    
    discard;
    return 0;
}