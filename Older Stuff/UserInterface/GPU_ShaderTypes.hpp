#pragma once

// DirectX 11
#include <DirectXMath.h>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec4.hpp"


namespace nui {

	inline DirectX::XMUINT2 toXM(glm::uvec2& value)
	{
		return { value.x, value.y };
	}

	inline DirectX::XMINT2 toXM(glm::ivec2& value)
	{
		return { value.x, value.y };
	}

	inline DirectX::XMINT2 toXM(int32_t x, int32_t y)
	{
		return { x, y };
	}

	inline DirectX::XMFLOAT2 toXM(glm::vec2& value)
	{
		return { value.x, value.y };;
	}

	inline DirectX::XMFLOAT4 toXM(glm::vec4& value)
	{
		return { value.x, value.y, value.z, value.w };
	}


	struct GPU_CharacterVertex {
		DirectX::XMINT2 pos;
		DirectX::XMFLOAT2 uv;
		uint32_t instance_id;
	};

	struct GPU_TextInstance {
		DirectX::XMFLOAT4 color;
	};


	struct GPU_SimpleVertex {
		DirectX::XMINT2 pos;
		uint32_t instance_id;
	};

	struct GPU_RectInstance {
		DirectX::XMFLOAT4 color;
	};

	struct GPU_CircleInstance {
		DirectX::XMFLOAT2 pos;
		float radius;

		DirectX::XMFLOAT4 color;
	};

	namespace GPU_ConstantsFields {
		enum {
			SCREEN_WIDTH,
			SCREEN_HEIGHT
		};
	}
}