
struct PixelInput
{
	float4 pixel_pos : SV_POSITION;
};

cbuffer Commons : register(b0)
{
	float4 pos_size;
	float4 colors[8];
	float4 color_pos[8];
	float gradient_angle;
};

float4 main(PixelInput input) : SV_Target
{
	return colors[0];
}