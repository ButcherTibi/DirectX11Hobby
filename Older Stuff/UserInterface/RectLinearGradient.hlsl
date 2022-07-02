
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

float2 rotate(float2 pos, float angle, float2 pivot)
{
	pos -= pivot;
	
	float2x2 rotation_mat = float2x2(
		cos(angle), -sin(angle),
		sin(angle), cos(angle));
	
	pos = mul(rotation_mat, pos);
	
	return pos + pivot;
}

float lerpFactor(float min, float mid, float max)
{
	return (mid - min) / (max - min);
}

float4 main(PixelInput input) : SV_TARGET
{
	float2 local_uv = (input.pixel_pos.xy - pos_size.xy) / pos_size.zw;
	
	local_uv = rotate(local_uv, -gradient_angle, float2(0.5, 0.5));
	local_uv.y = clamp(local_uv.y, 0, 1);  // diagonal is larger that 1
	
	for (uint i = 1; i < 8; i++) {
		
		float color_pos_prev = color_pos[i - 1].x;
		float color_pos_now  = color_pos[i].x;
		
		if (local_uv.y <= color_pos_now) {
			float a = lerpFactor(color_pos_prev, local_uv.y, color_pos_now);
			return lerp(colors[i - 1], colors[i], a);
		}
	}
	
	return float4(0, 0, 0, 1);
}
