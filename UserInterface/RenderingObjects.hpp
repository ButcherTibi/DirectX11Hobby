#pragma once

// Standard
#include <vector>
#include <array>

// Mine
#include "DX11Wrapper.hpp"
#include "CommonProperties.hpp"
#include "CharacterAtlas.hpp"


namespace nui {

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

	class RectRender {
	public:
		Window* window;

		std::vector<RectInstance> instances_props;

		std::vector<GPU_SimpleVertex> verts;
		dx11::Buffer vbuff;

		std::vector<uint32_t> indexes;
		dx11::Buffer idxbuff;

		std::vector<GPU_RectInstance> instances;
		dx11::Buffer sbuff;
		ComPtr<ID3D11ShaderResourceView> sbuff_srv;

	public:
		void init(Window* window);

		void reset();

		void addInstance(RectInstance& props);

		void generateGPU_Data();

		void draw();
	};


	struct TextInstance {
		int32_t pos[2];
		std::string text;
		std::string font_family;
		std::string font_style;
		uint32_t font_size;
		uint32_t line_height;
		Color color;
	};

	class TextRender {
	public:
		Window* window;

		struct InstanceProps {
			struct PositionedCharacter {
				std::array<int32_t, 2> pos;
				Character* chara;
			};

			std::vector<PositionedCharacter> chars;
			Color color;
		};
		std::vector<InstanceProps> instances_props;

		std::vector<GPU_CharacterVertex> verts;
		dx11::Buffer vbuff;

		std::vector<uint32_t> indexes;
		dx11::Buffer idxbuff;

		std::vector<GPU_TextInstance> instances;
		dx11::Buffer sbuff;
		ComPtr<ID3D11ShaderResourceView> sbuff_srv;

	public:
		void init(Window* window);

		void reset();

		// What is the size of each character
		// Where it should be placed
		// How large the container should be
		void addInstance(TextInstance& props, uint32_t& r_width, uint32_t& r_height);

		void generateGPU_Data();

		void draw();
	};
}