
struct PixelInput
{
	float4 pixel_pos : SV_POSITION;
};

cbuffer Stub : register(b0)
{
	float4 pos_size;
	float4 colors[8];
	float4 color_pos[8];
	float gradient_angle;
};

float lerpFactor(float min, float mid, float max)
{
	return (mid - min) / (max - min);
}

float4 main(PixelInput input) : SV_TARGET
{	
	float pos_y = pos_size.y;
	float size_y = pos_size.w;
	float2 pix_pos = input.pixel_pos.xy;
	
	for (uint i = 1; i < 8; i++) {
		
		float color_pos_prev = pos_y + size_y * color_pos[i - 1].x;
		float color_pos_now = pos_y + size_y * color_pos[i].x;
		
		if (pix_pos.y < color_pos[i].x) {
			float a = lerpFactor(color_pos_prev, pix_pos.y, color_pos_now);
			return lerp(colors[i - 1], colors[i], a);
		}
	}
	
	return float4(0, 0, 0, 0);
}