#pragma once

// Standard
#include <list>
#include <vector>
#include <array>
#include <variant>
#include <string>

// GLM
#include <glm\vec2.hpp>
#include <glm\vec4.hpp>

// Mine
#include "ErrorStuff.h"
#include "TextureAtlas.h"
#include "GPU_ShaderTypes.h"


namespace ui {

	// Forward declare
	struct CharInstance;


	enum class SizeReference {
		MARGIN_BOX,
		BORDER_BOX,
		PADDING_BOX,
		CONTENT_BOX
	};

	enum class SizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE,
		FIT
	};

	struct ElemSize {
		SizeType type;
		float size;

		ElemSize();
		ElemSize(SizeType type, float size);
	};

	enum class ImageMapping {
		STRETCH,
		FIT,
		CUSTOM,
	};

	enum class ImageTiling {
		REPEAT,
		MIRRORED_REPEAT,
		CLAMP_TO_EDGE,
		CLAMP_TO_BORDER,
		MIRROR_CLAMP_TO_EDGE
	};

	enum class FlexCrossAxisAlign {
		START,
		END,
		CENTER,
		PARENT,
	};

	enum class Overflow {
		HIDE,
		LEAK,
		SCROLL,
	};

	struct BoxModel {
		SizeReference size_reference;

		// Content Size
		ElemSize width;
		ElemSize height;

		// Padding
		ElemSize padding_top;
		ElemSize padding_right;
		ElemSize padding_bot;
		ElemSize padding_left;

		float padding_tl_radius;
		float padding_tr_radius;
		float padding_br_radius;
		float padding_bl_radius;

		// Border
		ElemSize border_top;
		ElemSize border_right;
		ElemSize border_bot;
		ElemSize border_left;

		glm::vec4 border_top_color;
		glm::vec4 border_right_color;
		glm::vec4 border_bot_color;
		glm::vec4 border_left_color;

		float border_tl_radius;
		float border_tr_radius;
		float border_br_radius;
		float border_bl_radius;

		// Margin
		ElemSize margin_top;
		ElemSize margin_right;
		ElemSize margin_bot;
		ElemSize margin_left;

		// Background
		glm::vec4 background_color;
		glm::vec4 top_left_color;
		glm::vec4 top_right_color;
		glm::vec4 bot_right_color;
		glm::vec4 bot_left_color;

		// Gradient

		// Image
		// Image Asset Handle
		ImageMapping image_mapping;
		glm::vec2 top_left_uv;
		glm::vec2 top_right_uv;
		glm::vec2 bot_right_uv;
		glm::vec2 bot_left_uv;

		ImageTiling tiling_U;
		ImageTiling tiling_V;
		float scale_U;
		float scale_V;
		// image post processing

		// Blur

		// Video

		FlexCrossAxisAlign flex_cross_axis_align_self;
		float opacity;
		// blurring

		Overflow horizontal_overflow;
		Overflow vertical_overflow;

		// animations

		// Computed Content Box (if relative then recompute for rendering)
		glm::vec3 origin;
		float content_width;
		float content_height;

		// Computed Padding
		float padding_left_thick;
		float padding_right_thick;
		float paddingbox_width;

		float padding_top_thick;
		float padding_bot_thick;
		float paddingbox_height;

		// Computed Border
		float border_left_thick;
		float border_right_thick;
		float borderbox_width;

		float border_top_thick;
		float border_bot_thick;
		float borderbox_height;

		// Computed Margin
		float margin_left_thick;
		float margin_right_thick;
		float marginbox_width;

		float margin_top_thick;
		float margin_bot_thick;
		float marginbox_height;

		

	public:
		void calculate(BoxModel* parent_box, float ancestor_width, float ancestor_height,
			float* new_ancestor_width, float* new_ancestor_heigth);
		void recalculateWidthBoxes(float new_content_width);
		void recalculateHeightBoxes(float new_content_height);
	};


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

	struct Div {
		BoxModel box;

		FlexDirection flex_direction;
		FlexWrap flex_wrap;

		FlexAxisAlign flex_axis_align;
		FlexCrossAxisAlign flex_cross_axis_align;

		FlexLinesAlign flex_lines_align;

		// Computed
		std::vector<glm::vec2> child_positions;
	};


	enum class Position {
		MARGINS,
		CENTER,
	};

	struct Wrap {
		BoxModel box;

		Position position;
		bool left_defined;
		bool right_defined;
		bool top_defined;
		bool bot_defined;
		ElemSize top;
		ElemSize right;
		ElemSize bot;
		ElemSize left;

		// Computed
		glm::vec2 child_pos;
	};


	struct Paragraph {
		BoxModel box;

		std::vector<uint32_t> chars;

		glm::vec4 font_color;
		std::string font_family;
		std::string font_style;
		float font_size;

		float line_height_scale;
		bool wrap_text;

		// Internal
		std::vector<CharInstance*> char_insts;
	};


	struct Node {
		Node* parent;
		std::variant<Div, Wrap, Paragraph> elem;
		std::list<Node*> children;
	};


	struct CharInstance {
		glm::vec2 screen_pos;
		float scale;
	};

	struct CharRaster {
		uint32_t unicode;

		float width;
		float height;
		float baseline_height;
		float advance;  // width + padding left right

		BasicBitmap bitmap;
		Zone* zone;

		std::list<CharInstance> instances;
	};

	struct FontSize {
		uint32_t raster_size;

		uint32_t height;
		uint32_t ascender;
		uint32_t descender;

		std::vector<CharRaster> rasters;

	public:
		CharRaster* findCharUnicode(uint32_t unicode);
	};

	struct CharMesh {
		uint32_t unicode;

		std::array<glm::vec2, 4> verts;
	};

	struct Font {
		std::string font_family;
		std::string font_style;

		std::vector<FontSize> font_sizes;
		std::vector<CharMesh> meshes;
	};

	struct FontInfo {
		std::string family_name;
		std::string style_name;

		std::vector<uint32_t> sizes_px;
	};


	class UserInterface {
	public:
		std::vector<Font> fonts;

		TextureAtlas atlas;

		std::list<Node> nodes;

	private:
		FontSize* findBestFitFontSize(std::string font_family, std::string font_style, float font_size);

	public:
		// add bitmap and set verts
		ErrStack addFont(std::vector<uint8_t>& font_ttf, FontInfo& info);

		ErrStack rebindToAtlas(uint32_t atlas_size);

		void recreateGraph(float screen_width, float screen_height);

		template<typename T>
		Node* addNode(Node* parent, T& new_elem);

		void deleteNode(Node* node, std::vector<Node*>& detached_nodes);
		/*void deleteGraph(Node* root);
		void detachGraph(Node* root);*/

		ErrStack calcElementBox(Node* elem, float container_width, float container_height, BoxModel* parent_box,
			BoxModel*& r_box);
	};
}
