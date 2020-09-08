// Header
#include "Internals.h"


using namespace nui_int;
using namespace nui_old;


void ContentSize::setAbsolute(float size)
{
	this->type = ContentSizeType::ABSOLUTE_SIZE;
	this->size = size;
}

void ContentSize::setRelative(float size)
{
	this->type = ContentSizeType::RELATIVE_SIZE;
	this->size = size;
}

void PropertySize::setAbsolute(float size)
{
	this->type = SizeType::ABSOLUTE_SIZE;
	this->size = size;
}

void PropertySize::setRelative(float size)
{
	this->type = SizeType::RELATIVE_SIZE;
	this->size = size;
}

void BoxModel::_calculateBoxModel(float& ancestor_width, float& ancestor_height)
{
	// Calculate Thicknesses
	auto calcSize = [](PropertySize size, float size_ref) {

		if (size.type == SizeType::RELATIVE_SIZE) {
			return size_ref * size.size;
		}
		return size.size;
	};

	_padding_left_thick = calcSize(padding_left, ancestor_width);
	_padding_right_thick = calcSize(padding_right, ancestor_width);
	_padding_top_thick = calcSize(padding_top, ancestor_height);
	_padding_bot_thick = calcSize(padding_bot, ancestor_height);

	_border_left_thick = calcSize(border_left, ancestor_width);
	_border_right_thick = calcSize(border_right, ancestor_width);
	_border_top_thick = calcSize(border_top, ancestor_height);
	_border_bot_thick = calcSize(border_bot, ancestor_height);

	// Calculate Boxes
	switch (box_sizing) {
	case BoxSizing::CONTENT: {

		switch (width.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_contentbox_width = width.size;
			_paddingbox_width = _padding_left_thick + _contentbox_width + _padding_right_thick;
			_borderbox_width = _border_left_thick + _paddingbox_width + _border_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE: {

			_contentbox_width = width.size * ancestor_width;
			_paddingbox_width = _padding_left_thick + _contentbox_width + _padding_right_thick;
			_borderbox_width = _border_left_thick + _paddingbox_width + _border_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::FIT:
			ancestor_width = ancestor_width;
			break;
		}

		switch (height.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_contentbox_height = height.size;
			_paddingbox_height = _padding_top_thick + _contentbox_height + _padding_bot_thick;
			_borderbox_height = _border_top_thick + _paddingbox_height + _border_bot_thick;
			ancestor_height = _contentbox_height;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE:

			_contentbox_height = height.size * ancestor_height;
			_paddingbox_height = _padding_top_thick + _contentbox_height + _padding_bot_thick;
			_borderbox_height = _border_top_thick + _paddingbox_height + _border_bot_thick;
			ancestor_height = _contentbox_height;
			break;

			// Unable to have any boxes
		case ContentSizeType::FIT:
			ancestor_height = ancestor_height;
			break;
		}
		break;
	}

	case BoxSizing::PADDING: {

		switch (width.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_paddingbox_width = width.size;
			_contentbox_width = _paddingbox_width - _padding_left_thick - _padding_right_thick;
			_borderbox_width = _border_left_thick + _paddingbox_width + _border_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE: {

			_paddingbox_width = width.size * ancestor_width;
			_contentbox_width = _paddingbox_width - _padding_left_thick - _padding_right_thick;
			_borderbox_width = _border_left_thick + _paddingbox_width + _border_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::FIT:
			ancestor_width = ancestor_width;
			break;
		}

		switch (height.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_paddingbox_height = height.size;
			_contentbox_height = _paddingbox_height - _padding_top_thick - _padding_bot_thick;
			_borderbox_height = _border_top_thick + _paddingbox_height + _border_bot_thick;
			ancestor_height = _contentbox_height;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE: {

			_paddingbox_height = height.size * ancestor_height;
			_contentbox_height = _paddingbox_height - _padding_top_thick - _padding_bot_thick;
			_borderbox_height = _border_top_thick + _paddingbox_height + _border_bot_thick;
			ancestor_height = _contentbox_height;
			break;
		}
		case ContentSizeType::FIT:
			ancestor_height = ancestor_height;
			break;
		}
		break;
	}

	case BoxSizing::BORDER: {

		switch (width.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_borderbox_width = width.size;
			_paddingbox_width = _borderbox_width - _border_left_thick - _border_right_thick;
			_contentbox_width = _paddingbox_width - _padding_left_thick - _padding_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE: {

			_borderbox_width = width.size * ancestor_width;
			_paddingbox_width = _borderbox_width - _border_left_thick - _border_right_thick;
			_contentbox_width = _paddingbox_width - _padding_left_thick - _padding_right_thick;
			ancestor_width = _contentbox_width;
			break;
		}
		case ContentSizeType::FIT:
			ancestor_width = ancestor_width;
			break;
		}

		switch (height.type) {
		case ContentSizeType::ABSOLUTE_SIZE: {

			_borderbox_height = height.size;
			_paddingbox_height = _borderbox_height - _border_left_thick - _border_right_thick;
			_contentbox_height = _paddingbox_height - _padding_left_thick - _padding_right_thick;
			ancestor_height = _contentbox_height;
			break;
		}
		case ContentSizeType::RELATIVE_SIZE: {

			_borderbox_height = height.size * ancestor_height;
			_paddingbox_height = _borderbox_height - _border_left_thick - _border_right_thick;
			_contentbox_height = _paddingbox_height - _padding_left_thick - _padding_right_thick;
			ancestor_height = _contentbox_height;
			break;
		}
		case ContentSizeType::FIT:
			ancestor_height = ancestor_height;
			break;
		}
		break;
	}
	}

	// Clamp Corner Radiuses or Trim them
	auto clampAbove = [](float radius, float size) {
		if (radius > size) {
			return size;
		}
		return radius;
	};

	float max_size = _borderbox_width / 2;
	if (_borderbox_width > _borderbox_height) {
		max_size = _borderbox_height / 2;
	}

	if (_border_top_thick || _border_left_thick) {
		_border_tl_radius = clampAbove(border_tl_radius, max_size);
	}
	else {
		_border_tl_radius = max_size;
	}

	if (_border_top_thick || _border_right_thick) {
		_border_tr_radius = clampAbove(border_tr_radius, max_size);
	}
	else {
		_border_tr_radius = max_size;
	}

	if (_border_bot_thick || _border_right_thick) {
		_border_br_radius = clampAbove(border_br_radius, max_size);
	}
	else {
		_border_br_radius = max_size;
	}

	if (_border_bot_thick || _border_left_thick) {
		_border_bl_radius = clampAbove(border_bl_radius, max_size);
	}
	else {
		_border_bl_radius = max_size;
	}

	max_size = _paddingbox_width / 2;
	if (_paddingbox_width > _paddingbox_height) {
		max_size = _paddingbox_height / 2;
	}

	_padding_tl_radius = clampAbove(padding_tl_radius, max_size);
	_padding_tr_radius = clampAbove(padding_tr_radius, max_size);
	_padding_br_radius = clampAbove(padding_br_radius, max_size);
	_padding_bl_radius = clampAbove(padding_bl_radius, max_size);
}

void BoxModel::_recalculateWidthBoxes(float new_content_width)
{
	_contentbox_width = new_content_width;
	_paddingbox_width = _padding_left_thick + _contentbox_width + _padding_right_thick;
	_borderbox_width = _border_left_thick + _paddingbox_width + _border_right_thick;
}

void BoxModel::_recalculateHeightBoxes(float new_content_height)
{
	_contentbox_height = new_content_height;
	_paddingbox_height = _padding_top_thick + _contentbox_height + _padding_bot_thick;
	_borderbox_height = _border_top_thick + _paddingbox_height + _border_bot_thick;
}

Element& UserInterface::getRoot()
{
	return this->elems.front();
}

Wrap& UserInterface::getRootElement()
{
	Element& elem = this->elems.front();
	return std::get<Wrap>(elem.elem);
}

template<typename T>
Element& UserInterface::addElement(Element& parent, T& new_elem)
{
	Element& new_node = elems.emplace_back();
	new_node.parent = &parent;
	new_node.elem = new_elem;

	parent.children.push_back(&new_node);

	return new_node;
}
template Element& UserInterface::addElement(Element& parent, Wrap& new_elem);

struct Line {
	size_t end;
	float cross_size;
};

struct Word {
	size_t end;
	float advance;
};

void UserInterface::_calcElementLayout(Element* elem, uint32_t parent_layer_idx, float ancestor_width, float ancestor_height,
	BoxModel*& r_box)
{
	// Register this element to layer
	uint32_t idx = elem->parent != nullptr ? parent_layer_idx + 1 : 0;

	ElementsLayer* layer;
	if (idx == this->layers.size()) {
		layer = &this->layers.emplace_back();
	}
	else {
		auto it = this->layers.begin();
		std::advance(it, idx);
		layer = &(*it);
	}
	layer->elems.push_back(elem);

	// Distinct properties
	auto flex = std::get_if<Wrap>(&elem->elem);
	if (flex != nullptr) {

		flex->_calculateBoxModel(ancestor_width, ancestor_height);

		// skip if no children
		if (!elem->children.size()) {
			r_box = flex;
			return;
		}

		// Child origin
		glm::vec2 child_origin = flex->_origin;
		child_origin.x += flex->_padding_left_thick +  flex->_border_left_thick;
		child_origin.y += flex->_padding_top_thick + flex->_border_top_thick;

		// Child Boxes
		std::vector<BoxModel*> child_boxes(elem->children.size());
		size_t child_idx = 0;

		for (Element* child : elem->children) {

			_calcElementLayout(child, idx, ancestor_width, ancestor_height,
				child_boxes[child_idx]);

			child_boxes[child_idx]->_origin = child_origin;

			child_idx++;
		}

		// Calc Children Layout
		auto calcRowLayout = [&](size_t start, size_t end,
			float used_width, float width, float height)
		{
			auto packChildren = [&](float start_x, float step) {
				for (size_t i = start; i < end; i++) {

					BoxModel* child_box = child_boxes[i];
					child_box->_origin.x += start_x;

					switch (child_box->flex_cross_axis_align_self) {
					case FlexCrossAxisAlign::START:
						child_box->_origin.y += 0;
						break;
					case FlexCrossAxisAlign::END:
						child_box->_origin.y += height - child_box->_borderbox_height;
						break;
					case FlexCrossAxisAlign::CENTER:
						child_box->_origin.y += (height - child_box->_borderbox_height) / 2;
						break;

					case FlexCrossAxisAlign::PARENT:
						switch (flex->cross_axis_align) {
						case FlexCrossAxisAlign::START:
							child_box->_origin.y += 0;
							break;
						case FlexCrossAxisAlign::END:
							child_box->_origin.y += height - child_box->_borderbox_height;
							break;
						case FlexCrossAxisAlign::CENTER:
							child_box->_origin.y += (height - child_box->_borderbox_height) / 2;
							break;
						}
						break;
					}

					start_x += child_box->_borderbox_width + step;
				}
			};

			switch (flex->axis_align) {
			case FlexAxisAlign::START:
				packChildren(0, 0);
				break;

			case FlexAxisAlign::END:
				packChildren(width - used_width, 0);
				break;

			case FlexAxisAlign::CENTER:
				packChildren((width - used_width) / 2, 0);
				break;

			case FlexAxisAlign::SPACE_BETWEEN:
				packChildren(0, (width - used_width) / (end - start - 1));
			}
		};

		auto calcRowLines = [&](float used_height, float height, std::vector<Line>& lines) {

			auto pack = [&](float line_start, float step) {

				size_t i = 0;
				for (Line& line : lines) {

					for (; i < line.end; i++) {

						BoxModel* box = child_boxes[i];
						box->_origin.y += line_start;
					}

					line_start += line.cross_size + step;
				}
			};

			switch (flex->lines_align) {
			case FlexLinesAlign::START:
				pack(0, 0);
				break;

			case FlexLinesAlign::END:
				pack(height - used_height, 0);
				break;

			case FlexLinesAlign::CENTER:
				pack((height - used_height) / 2, 0);
				break;

			case FlexLinesAlign::SPACE_BETWEEN:
				pack(0, (height - used_height) / (lines.size() - 1));
				break;
			}
		};

		auto calcColumLayout = [&](size_t start, size_t end,
			float used_height, float height, float width)
		{
			auto pack = [&](float start_y, float step) {
				for (size_t i = start; i < end; i++) {

					BoxModel* child_box = child_boxes[i];
					child_box->_origin.y += start_y;

					switch (child_box->flex_cross_axis_align_self) {
					case FlexCrossAxisAlign::START:
						child_box->_origin.x += 0;
						break;
					case FlexCrossAxisAlign::END:
						child_box->_origin.x += width - child_box->_borderbox_width;
						break;
					case FlexCrossAxisAlign::CENTER:
						child_box->_origin.x += (width - child_box->_borderbox_width) / 2;
						break;

					case FlexCrossAxisAlign::PARENT:
						switch (flex->cross_axis_align) {
						case FlexCrossAxisAlign::START:
							child_box->_origin.x += 0;
							break;
						case FlexCrossAxisAlign::END:
							child_box->_origin.x += width - child_box->_borderbox_width;
							break;
						case FlexCrossAxisAlign::CENTER:
							child_box->_origin.x += (width - child_box->_borderbox_width) / 2;
							break;
						}
						break;
					}

					start_y += child_box->_borderbox_height + step;
				}
			};

			switch (flex->axis_align) {
			case FlexAxisAlign::START:
				pack(0, 0);
				break;

			case FlexAxisAlign::END:
				pack(height - used_height, 0);
				break;

			case FlexAxisAlign::CENTER:
				pack((height - used_height) / 2, 0);
				break;

			case FlexAxisAlign::SPACE_BETWEEN:
				pack(0, (height - used_height) / (end - start - 1));
			}
		};

		auto calcColumLines = [&](float used_width, float width, std::vector<Line>& lines) {

			auto pack = [&](float line_start, float step) {

				size_t i = 0;
				for (Line& line : lines) {

					for (; i < line.end; i++) {

						BoxModel* box = child_boxes[i];
						box->_origin.x += line_start;
					}

					line_start += line.cross_size + step;
				}
			};

			switch (flex->lines_align) {
			case FlexLinesAlign::START:
				pack(0, 0);
				break;

			case FlexLinesAlign::END:
				pack(width - used_width, 0);
				break;

			case FlexLinesAlign::CENTER:
				pack((width - used_width) / 2, 0);
				break;

			case FlexLinesAlign::SPACE_BETWEEN:
				pack(0, (width - used_width) / (lines.size() - 1));
				break;
			}
		};

		std::vector<Line> lines;
		lines.reserve(child_boxes.size() / 2);

		switch (flex->direction) {
		case FlexDirection::ROW: {

			switch (flex->wrap) {
			case FlexWrap::NO_WRAP: {

				float used_width = 0;
				float row_height = 0;

				for (BoxModel* child_box : child_boxes) {

					used_width += child_box->_borderbox_width;

					if (child_box->_borderbox_height > row_height) {
						row_height = child_box->_borderbox_height;
					}
				}

				calcRowLayout(0, child_boxes.size(),
					used_width, ancestor_width, row_height);

				lines.emplace_back();
				lines[0].cross_size = row_height;
				lines[0].end = child_boxes.size();

				calcRowLines(row_height, ancestor_height, lines);

				if (flex->width.type == ContentSizeType::FIT) {
					flex->_recalculateWidthBoxes(used_width);
				}
				if (flex->height.type == ContentSizeType::FIT) {
					flex->_recalculateHeightBoxes(row_height);
				}
				break;
			}

			case FlexWrap::WRAP: {

				float lines_width = 0;  // maximum row width
				float lines_height = 0;  // height of all rows combined

				float used_width = 0;
				float row_height = 0;
				uint32_t start = 0;

				for (uint32_t i = 0; i < child_boxes.size(); i++) {

					BoxModel* box = child_boxes[i];

					used_width += box->_borderbox_width;

					if (box->_borderbox_height > row_height) {
						row_height = box->_borderbox_height;
					}

					if (i == child_boxes.size() - 1 ||
						(used_width + child_boxes[i + 1]->_borderbox_width > ancestor_width && i > 0))
					{
						calcRowLayout(start, i + 1, used_width, ancestor_width, row_height);

						Line& line = lines.emplace_back();
						line.cross_size = row_height;
						line.end = i + 1;

						lines_height += row_height;

						used_width = 0;
						row_height = 0;
						start = i + 1;
					}

					if (used_width > lines_width) {
						lines_width = used_width;
					}
				}

				calcRowLines(lines_height, ancestor_height, lines);

				if (flex->width.type == ContentSizeType::FIT) {
					flex->_recalculateWidthBoxes(lines_width);
				}
				if (flex->height.type == ContentSizeType::FIT) {
					flex->_recalculateHeightBoxes(lines_height);
				}
				break;
			}
			}
			break;
		}
		case FlexDirection::COLUMN: {

			switch (flex->wrap) {
			case FlexWrap::NO_WRAP: {

				float used_height = 0;
				float col_width = 0;

				for (BoxModel* child_box : child_boxes) {

					used_height += child_box->_borderbox_height;

					if (child_box->_borderbox_width > col_width) {
						col_width = child_box->_borderbox_width;
					}
				}

				calcColumLayout(0, child_boxes.size(),
					used_height, ancestor_height, col_width);

				lines.emplace_back();
				lines[0].cross_size = col_width;
				lines[0].end = child_boxes.size();

				calcColumLines(col_width, ancestor_width, lines);

				if (flex->width.type == ContentSizeType::FIT) {
					flex->_recalculateWidthBoxes(col_width);
				}
				if (flex->height.type == ContentSizeType::FIT) {
					flex->_recalculateHeightBoxes(used_height);
				}
				break;
			}
			case FlexWrap::WRAP: {

				float lines_width = 0;  // maximum row width
				float lines_height = 0;  // height of all rows combined

				float used_height = 0;
				float col_width = 0;
				uint32_t start = 0;

				for (uint32_t i = 0; i < child_boxes.size(); i++) {

					BoxModel* box = child_boxes[i];

					used_height += box->_borderbox_height;

					if (box->_borderbox_width > col_width) {
						col_width = box->_borderbox_width;
					}

					if (i == child_boxes.size() - 1 ||
						(used_height + child_boxes[i + 1]->_borderbox_height > ancestor_height && i > 0))
					{
						calcColumLayout(start, i + 1, used_height, ancestor_height, col_width);

						Line& line = lines.emplace_back();
						line.cross_size = col_width;
						line.end = i + 1;

						lines_width += col_width;

						used_height = 0;
						col_width = 0;
						start = i + 1;
					}

					if (used_height > lines_height) {
						used_height = used_height;
					}
				}

				calcColumLines(lines_width, ancestor_width, lines);

				if (flex->width.type == ContentSizeType::FIT) {
					flex->_recalculateWidthBoxes(lines_width);
				}
				if (flex->height.type == ContentSizeType::FIT) {
					flex->_recalculateHeightBoxes(lines_height);
				}
				break;
			}
			}
			break;
		}
		}

		r_box = flex;
		return;
	}

	//auto par = std::get_if<Paragraph>(&elem->elem);
	//if (par != nullptr) {

	//	par->_calculateBoxModel(ancestor_width, ancestor_height);

	//	// Font
	//	FontSize* font = _findBestFitFontSize(par->font_family, par->font_style, par->font_size);
	//	float scale_unit = par->font_size / font->raster_size;
	//	float line_ascender = font->ascender * par->line_height * scale_unit;
	//	float line_descender = font->descender * par->line_height * scale_unit;
	//	float line_height = font->height * par->line_height * scale_unit;  // ascender + descender
	//	float line_width = 0;

	//	float text_width = 0;

	//	glm::vec2 pen_pos = par->_origin;
	//	pen_pos.y += line_ascender;

	//	switch (par->word_wrap) {
	//	case WordWrap::NONE: {

	//		for (uint32_t i = 0; i < par->chars.size(); i++) {

	//			// if char is LF line feed
	//			if (par->chars[i] == 0x00A) {
	//				pen_pos.x = 0;
	//				pen_pos.y += line_height;
	//			}

	//			CharRaster* raster = font->findCharUnicode(par->chars[i]);

	//			float advance = raster->advance * scale_unit;
	//			float descender = (raster->height - raster->baseline_height) * scale_unit;

	//			CharInstance& char_inst = raster->instances.emplace_back();
	//			char_inst.screen_pos = pen_pos;
	//			char_inst.screen_pos.y += descender;  // "agf" g needs to be positioned lower on the line
	//			char_inst.scale = scale_unit;

	//			pen_pos.x += advance;

	//			if (pen_pos.x > text_width) {
	//				text_width = pen_pos.x;
	//			}
	//		}
	//		break;
	//	}
	//	}

	//	//// Group Characters into words
	//	//std::vector<CharRaster*> char_rasters;
	//	//std::vector<Word> words;
	//	//{
	//	//	char_rasters.resize(par->chars.size());
	//	//	words.reserve(par->chars.size() / 3);  // heuristic

	//	//	Word word = {};
	//	//	for (uint32_t i = 0; i < par->chars.size(); i++) {

	//	//		CharRaster* raster = font->findCharUnicode(par->chars[i]);
	//	//		char_rasters[i] = raster;

	//	//		word.end = i;

	//	//		if (par->chars[i] == ' ') {

	//	//			words.push_back(word);
	//	//			word = {};
	//	//		}
	//	//		else {
	//	//			word.advance += raster->advance * scale_unit;
	//	//		}
	//	//	}
	//	//}
	//	//
	//	//float line_width = 0;

	//	//size_t start = 0;
	//	//for (Word& word : words) {

	//	//	if (line_width + word.advance > ancestor_width) {

	//	//	}
	//	//	else {
	//	//		for (size_t i = start; i < word.end; i++) {

	//	//			CharRaster* raster = char_rasters[i];

	//	//			CharInstance& char_inst = raster->instances.emplace_back();
	//	//			char_inst.screen_pos = pen_pos;
	//	//		}
	//	//	}
	//	//}

	//	if (par->width.type == ContentSizeType::FIT) {
	//		par->recalculateWidthBoxes(text_width);
	//	}
	//	if (par->height.type == ContentSizeType::FIT) {
	//		par->recalculateHeightBoxes(pen_pos.y + line_descender);
	//	}

	//	r_box = par;
	//	return;
	//}
}

void UserInterface::calcGraphLayout()
{
	this->layers.clear();

	BoxModel* root_box;
	_calcElementLayout(&elems.front(), 0, 0, 0, root_box);
}
