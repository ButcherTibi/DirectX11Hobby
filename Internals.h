#pragma once

// Standard
#include <array>
#include <list>
#include <variant>

// Vulkan
#include "VulkanRender.h"

// Mine
#include "ErrorStuff.h"


namespace nui {
	struct Color {
		float r, g, b, a;
	};

	enum class BoxSizing {
		CONTENT,
		PADDING,
		BORDER,
	};

	enum class ContentSizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE,
		FIT
	};

	struct ContentSize {
		ContentSizeType type = ContentSizeType::FIT;
		float size;

		void setAbsolute(float size);
		void setRelative(float size);
	};

	enum class SizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE
	};

	struct PropertySize {
		SizeType type = SizeType::ABSOLUTE_SIZE;
		float size = 0;

		void setAbsolute(float size);
		void setRelative(float size);
	};

	enum class FlexCrossAxisAlign {
		START,
		END,
		CENTER,
		PARENT,
	};

	struct BoxModel {
		// Box Size
		BoxSizing box_sizing = BoxSizing::CONTENT;
		ContentSize width;
		ContentSize height;

		// Padding
		PropertySize padding_top;
		PropertySize padding_right;
		PropertySize padding_bot;
		PropertySize padding_left;

		float padding_tl_radius;
		float padding_tr_radius;
		float padding_br_radius;
		float padding_bl_radius;

		// Border
		PropertySize border_top;
		PropertySize border_right;
		PropertySize border_bot;
		PropertySize border_left;

		float border_tl_radius;
		float border_tr_radius;
		float border_br_radius;
		float border_bl_radius;

		// Background
		Color background_color = { 0, 0, 0, 1 };
		Color border_color = { 0, 0, 0, 1 };

		// Children
		FlexCrossAxisAlign flex_cross_axis_align_self = FlexCrossAxisAlign::PARENT;

		// Computed Content Box (if relative then recompute for rendering)
		glm::vec2 _origin;

		float _contentbox_width;
		float _contentbox_height;

		// Computed Padding
		float _padding_left_thick;
		float _padding_right_thick;
		float _paddingbox_width;

		float _padding_top_thick;
		float _padding_bot_thick;
		float _paddingbox_height;

		float _padding_tl_radius;
		float _padding_tr_radius;
		float _padding_br_radius;
		float _padding_bl_radius;

		// Computed Border
		float _border_left_thick;
		float _border_right_thick;
		float _borderbox_width;

		float _border_top_thick;
		float _border_bot_thick;
		float _borderbox_height;

		float _border_tl_radius;
		float _border_tr_radius;
		float _border_br_radius;
		float _border_bl_radius;

	public:
		void _calculateBoxModel(float& ancestor_width, float& ancestor_height);
		void _recalculateWidthBoxes(float new_content_width);
		void _recalculateHeightBoxes(float new_content_height);
	};

	// TODO:
	// - layout
	// - image
	// - gradients
	// - CSS styling
	// - interaction
	// - animation

	//enum class Position {
	//	MARGINS,
	//	CENTER,
	//};

	//struct Wrap {
	//	BoxModel box;

	//	Position position;
	//	bool left_defined;
	//	bool right_defined;
	//	bool top_defined;
	//	bool bot_defined;
	//	ElemSize top;
	//	ElemSize right;
	//	ElemSize bot;
	//	ElemSize left;

	//	// Computed
	//	glm::vec2 child_pos;
	//};

	enum class FlexDirection {
		ROW,
		COLUMN,
	};

	enum class FlexWrap {
		NO_WRAP,
		WRAP,
	};

	enum class FlexAxisAlign {
		START,
		END,
		CENTER,
		SPACE_BETWEEN,
	};

	enum class FlexLinesAlign {
		START,
		END,
		CENTER,
		SPACE_BETWEEN,
	};

	struct Flex : BoxModel {
		FlexDirection direction = FlexDirection::ROW;
		FlexWrap wrap = FlexWrap::NO_WRAP;
		FlexAxisAlign axis_align = FlexAxisAlign::START;
		FlexCrossAxisAlign cross_axis_align = FlexCrossAxisAlign::START;
		FlexLinesAlign lines_align = FlexLinesAlign::START;
	};


	enum class WordWrap {
		NONE,
		LETTER,
		WORD,
	};

	struct Paragraph : BoxModel {
		std::vector<uint32_t> chars;
		std::string font_family;
		std::string font_style;
		float font_size;
		Color font_color;

		float line_height;
		WordWrap word_wrap;
	};

	struct Element {
		Element* parent;
		std::variant<Flex, Paragraph> elem;
		std::list<Element*> children;
	};
}

namespace nui_int {

	struct ElementsLayer {
		std::list<nui::Element*> elems;
	};

	class UserInterface {
	public:
		std::list<nui::Element> elems;
		std::list<ElementsLayer> layers;

	public:
		nui::Element& getRoot();
		nui::Flex& getRootElement();

		template<typename T>
		nui::Element& addElement(nui::Element& parent, T& new_elem);

		void _calcElementLayout(nui::Element* elem, uint32_t parent_layer_idx, float ancestor_width, float ancestor_height,
			nui::BoxModel*& r_box);

		void calcGraphLayout();
	};


	class Internals {
	public:
		UserInterface user_interface;
		VulkanRenderer vkr;

	public:
		ErrStack create(HWND hwnd, HINSTANCE hinstance);

		ErrStack generateGPU_Data();

		ErrStack draw();
	};
}
