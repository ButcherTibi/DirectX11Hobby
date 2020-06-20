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

enum class SizeTypes {
	RELATIVE_SIZE,
	ABSOLUTE_SIZE
};

struct PropertySize {
	SizeTypes type = SizeTypes::ABSOLUTE_SIZE;
	float size = 0;

	void setAbsolute(float size);
	void setRelative(float size);
};

//enum class FlexCrossAxisAlign {
//	START,
//	END,
//	CENTER,
//	PARENT,
//};

struct BoxModel {
	// Content Size
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

	// Computed Content Box (if relative then recompute for rendering)
	glm::vec2 origin_;

	float _contentbox_width;
	float _contentbox_height;

	// Computed Padding
	float padding_left_thick_;
	float padding_right_thick_;
	float paddingbox_width_;

	float padding_top_thick_;
	float padding_bot_thick_;
	float paddingbox_height_;

	// Computed Border
	float border_left_thick_;
	float border_right_thick_;
	float borderbox_width_;

	float border_top_thick_;
	float border_bot_thick_;
	float borderbox_height_;

public:
	void calculateBoxModel();
	void calculateBoxModel(float& ancestor_width, float& ancestor_height);
	void recalculateWidthBoxes(float new_content_width);
	void recalculateHeightBoxes(float new_content_height);
};


//enum class FlexDirection {
//	ROW,
//	COLUMN,
//};

//enum class FlexWrap {
//	NO_WRAP,
//	WRAP,
//};

//enum class FlexAxisAlign {
//	START,
//	END,
//	CENTER,
//	SPACE_BETWEEN,
//};

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

struct BasicElement : BoxModel {
	// test relative border size
	// clamp padding and border radius
	// test opacity
};


struct Element {
	Element* parent;
	std::variant<BasicElement> elem;
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
