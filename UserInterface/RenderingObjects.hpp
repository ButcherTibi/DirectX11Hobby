#pragma once

// Standard
#include <vector>
#include <array>

// Mine
#include "DX11Wrapper.hpp"
#include "CommonProperties.hpp"
#include "CharacterAtlas.hpp"


namespace nui {

	// Forward
	class Window;


	//struct RectGradientInstance {
	//	std::array<int32_t, 2> screen_pos;
	//	std::array<uint32_t, 2> size;

	//	BackgroundColoring coloring;
	//	std::vector<ColorStep> colors;
	//	float gradient_angle;  // degrees
	//};

	//// Renders a rectangle filled with a color gradient
	//class RectGradientRender {
	//public:
	//	Window* window;

	//	struct InstanceProps {
	//		BackgroundColoring coloring;

	//		std::array<int32_t, 2> screen_pos;
	//		std::array<uint32_t, 2> size;
	//		std::array<DirectX::XMFLOAT4, 8> colors;
	//		std::array<float, 8> color_lenghts;
	//		float gradient_angle;  // radians
	//	};
	//	std::vector<InstanceProps> instances_props;
	//	
	//	std::vector<GPU_SimpleVertex> verts;
	//	dx11::Buffer vbuff;

	//	std::vector<uint32_t> indexes;
	//	dx11::Buffer idxbuff;

	//	dx11::ConstantBuffer gradient_rect_cbuff;  // TODO: try replace with structured

	//	// Drawcall
	//	uint32_t vertex_start_idx;
	//	uint32_t vertex_count;
	//	uint32_t index_start_idx;
	//	uint32_t index_count;

	//public:
	//	void init(Window* window);

	//	void reset();

	//	void addInstance(RectGradientInstance& props);

	//	void generateGPU_Data();

	//	void draw();
	//};


	struct RectInstance {
		std::array<int32_t, 2> screen_pos;
		std::array<uint32_t, 2> size;

		Color color;
	};

	struct ArrowInstance {
		// TODO: arrow_type
		// TODO: direction

		std::array<int32_t, 2> screen_pos;
		std::array<uint32_t, 2> size;

		Color color;
	};

	struct BorderInstance {
		std::array<int32_t, 2> screen_pos;
		std::array<uint32_t, 2> size;
		uint32_t thickness;

		Color color;

		bool top = true;
		bool bot = true;
		bool left = true;
		bool right = true;
	};


	struct PositionedCharacter {
		std::array<int32_t, 2> pos;
		Character* chara;
	};

	struct TextInstance {
		std::vector<PositionedCharacter> chars;
		Color color;
	};
}