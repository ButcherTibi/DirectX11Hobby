
// Header
#include "GPU_ShaderTypes.hpp"


DirectX::XMUINT2 nui::toXM(glm::uvec2& value)
{
	return { value.x, value.y };
}

DirectX::XMINT2 nui::toXM(glm::ivec2& value)
{
	return { value.x, value.y };
}

DirectX::XMINT2 nui::toXM(int32_t x, int32_t y)
{
	return { x, y };
}

DirectX::XMFLOAT2 nui::toXM(glm::vec2& value)
{
	return { value.x, value.y };;
}

DirectX::XMFLOAT4 nui::toXM(glm::vec4& value)
{
	return { value.x, value.y, value.z, value.w };
}
