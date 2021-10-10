module;

// DirectX 11
#include <DirectXMath.h>

// GLM
#include "glm\vec2.hpp"
#include "glm\vec4.hpp"

export module GPU_ShaderTypes;


namespace nui {

	export DirectX::XMUINT2 toXM(glm::uvec2& value);
	export DirectX::XMINT2 toXM(glm::ivec2& value);
	export DirectX::XMINT2 toXM(int32_t x, int32_t y);
	export DirectX::XMFLOAT2 toXM(glm::vec2& value);
	export DirectX::XMFLOAT4 toXM(glm::vec4& value);


	export struct GPU_CharacterVertex {
		DirectX::XMINT2 pos;
		DirectX::XMFLOAT2 uv;
		uint32_t instance_id;
	};

	export struct GPU_TextInstance {
		DirectX::XMFLOAT4 color;
	};


	export struct GPU_SimpleVertex {
		DirectX::XMINT2 pos;
		uint32_t instance_id;
	};

	export struct GPU_RectInstance {
		DirectX::XMFLOAT4 color;
	};

	export struct GPU_CircleInstance {
		DirectX::XMFLOAT2 pos;
		float radius;

		DirectX::XMFLOAT4 color;
	};

	export namespace GPU_ConstantsFields {
		enum {
			SCREEN_WIDTH,
			SCREEN_HEIGHT
		};
	}


	/////////////////////////////////////////////////////////////////////////
	// Implementation 
	/////////////////////////////////////////////////////////////////////////

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
}