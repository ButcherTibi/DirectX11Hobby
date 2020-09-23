#include "pch.h"

// Header
#include "GPU_ShaderTypes.hpp"


DirectX::XMFLOAT2 nui::toXMFloat2(glm::vec2 val)
{
	DirectX::XMFLOAT2 dx2;
	dx2.x = val.x;
	dx2.y = val.y;
	return dx2;
}