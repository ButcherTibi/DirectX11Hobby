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


// Forward declare
struct CharInstance;


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
	glm::vec4 border_color;
	glm::vec4 background_color;

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
	void calculateBoxModel(float& ancestor_width, float& ancestor_height);
	void recalculateWidthBoxes(float new_content_width);  // needs redone
	void recalculateHeightBoxes(float new_content_height);  // needs redone
};

// TODO:
// - layout
// - image
// - gradients
// - CSS styling
// - interaction
// - animation

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

//enum class FlexLinesAlign {
//	START,
//	END,
//	CENTER,
//	SPACE_BETWEEN,
//};

//struct Div {
//	BoxModel box;

//	FlexDirection flex_direction;
//	FlexWrap flex_wrap;

//	FlexAxisAlign flex_axis_align;
//	FlexCrossAxisAlign flex_cross_axis_align;

//	FlexLinesAlign flex_lines_align;

//	// Computed
//	std::vector<glm::vec2> child_positions;
//};


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


//struct Paragraph {
//	BoxModel box;

//	std::vector<uint32_t> chars;

//	glm::vec4 font_color;
//	std::string font_family;
//	std::string font_style;
//	float font_size;

//	float line_height_scale;
//	bool wrap_text;

//	// Internal
//	std::vector<CharInstance*> char_insts;
//};

struct Flex : BoxModel {
	FlexDirection direction = FlexDirection::ROW;
	FlexWrap wrap = FlexWrap::NO_WRAP;
	FlexAxisAlign axis_align = FlexAxisAlign::START;
	FlexCrossAxisAlign cross_axis_align = FlexCrossAxisAlign::START;
};


struct Element {
	Element* parent;
	std::variant<Flex> elem;
	std::list<Element*> children;
};

struct ElementsLayer {
	std::list<Element*> elems;
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

	std::list<Element> elems;
	std::list<ElementsLayer> layers;

private:
	FontSize* findBestFitFontSize(std::string font_family, std::string font_style, float font_size);

public:
	// add bitmaps and set verts
	ErrStack addFont(std::vector<uint8_t>& font_ttf, FontInfo& info);

	ErrStack rebindToAtlas(uint32_t atlas_size);

	void recreateGraph(float screen_width, float screen_height);

	template<typename T>
	Element* addElement(Element* parent, T& new_elem);

	void deleteElement(Element* elem, std::vector<Element*>& detached_nodes);

	void calcElementLayout(Element* elem, uint32_t parent_layer_idx, float ancestor_width, float ancestor_height,
		BoxModel*& r_box);

	void calcGraphLayout();

	void changeResolution(float screen_width, float screen_height);
};
