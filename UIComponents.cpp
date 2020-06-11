
// Header
#include "UIComponents.h"


//struct Axis {
//	uint32_t idx_end;
//	float cross_axis_size;
//};

namespace ui {

	ElemSize::ElemSize()
	{
		this->type = SizeType::ABSOLUTE_SIZE;
		this->size = 0;
	}

	ElemSize::ElemSize(SizeType type, float size)
	{
		this->type = type;
		this->size = size;
	}

	void ElemSize::setAbsolute(float size)
	{
		this->type = SizeType::ABSOLUTE_SIZE;
		this->size = size;
	}

	static float calcSize(ElemSize size, float size_ref)
	{
		if (size.type == SizeType::RELATIVE_SIZE) {
			return size_ref * size.size;
		}
		return size.size;
	}

	void BoxModel::calculateBoxModel()
	{
		// Content
		content_width = width.size;
		content_height = height.size;

		// Padding
		padding_left_thick = 0;
		padding_right_thick = 0;
		padding_top_thick = 0;
		padding_bot_thick = 0;

		// Border
		border_left_thick = 0;
		border_right_thick = 0;
		border_top_thick = 0;
		border_bot_thick = 0;

		// Boxes
		paddingbox_width = padding_left_thick + content_width + padding_right_thick;
		borderbox_width = border_left_thick + paddingbox_width + border_right_thick;

		paddingbox_height = padding_top_thick + content_height + padding_bot_thick;
		borderbox_height = border_top_thick + paddingbox_height + border_bot_thick;
	}

	void BoxModel::calculateBoxModel(float& ancestor_width, float& ancestor_height)
	{
		// Content
		switch (width.type) {
		case SizeType::RELATIVE_SIZE:
			content_width = ancestor_width * width.size;
			break;

		case SizeType::ABSOLUTE_SIZE:
			content_width = width.size;
			break;

		case SizeType::FIT:
			content_width = ancestor_width;
			break;
		}

		switch (height.type) {
		case SizeType::RELATIVE_SIZE:
			content_height = ancestor_height * height.size;
			break;

		case SizeType::ABSOLUTE_SIZE:
			content_height = height.size;
			break;

		case SizeType::FIT:
			content_height = ancestor_height;
			break;
		}

		// Padding
		padding_left_thick = calcSize(padding_left, ancestor_width);
		padding_right_thick = calcSize(padding_right, ancestor_width);
		padding_top_thick = calcSize(padding_top, ancestor_height);
		padding_bot_thick = calcSize(padding_bot, ancestor_height);

		// Border
		border_left_thick = calcSize(border_left, ancestor_width);
		border_right_thick = calcSize(border_right, ancestor_width);
		border_top_thick = calcSize(border_top, ancestor_height);
		border_bot_thick = calcSize(border_bot, ancestor_height);
		
		// New Ancestor Sizes for children
		switch (width.type) {
		case SizeType::RELATIVE_SIZE:
		case SizeType::ABSOLUTE_SIZE:
			ancestor_width = content_width;

			paddingbox_width = padding_left_thick + content_width + padding_right_thick;
			borderbox_width = border_left_thick + paddingbox_width + border_right_thick;
			break;

		// Unable to have any boxes
		case SizeType::FIT:
			ancestor_width = ancestor_width;
			break;
		}

		switch (height.type) {
		case SizeType::RELATIVE_SIZE:
		case SizeType::ABSOLUTE_SIZE:
			ancestor_height = content_height;

			paddingbox_height = padding_top_thick + content_height + padding_bot_thick;
			borderbox_height = border_top_thick + paddingbox_height + border_bot_thick;
			break;

		// Unable to have any boxes
		case SizeType::FIT:
			ancestor_height = ancestor_height;
			break;
		}
	}

	void BoxModel::recalculateWidthBoxes(float new_content_width)
	{
		content_width = new_content_width;

		paddingbox_width = padding_left_thick + content_width + padding_right_thick;
		borderbox_width = border_left_thick + paddingbox_width + border_right_thick;
	}

	void BoxModel::recalculateHeightBoxes(float new_content_height)
	{
		content_height = new_content_height;

		paddingbox_height = padding_top_thick + content_height + padding_bot_thick;
		borderbox_height = border_top_thick + paddingbox_height + border_bot_thick;
	}

	//ErrStack UserInterface::calcElementBox(Node* elem, float ancestor_width, float ancestor_height, BoxModel* parent_box, 
	//	BoxModel*& r_box)
	//{
	//	ErrStack err;

	//	Div* div = std::get_if<Div>(&elem->elem);
	//	if (div != nullptr) {

	//		BoxModel& box = div->box;

	//		// Calculate Box
	//		float new_ancestor_width;
	//		float new_ancestor_height;
	//		box.calculate(parent_box, ancestor_width, ancestor_height, 
	//			&new_ancestor_width, &new_ancestor_height);

	//		if (!elem->children.size()) {
	//			return ErrStack();
	//		}

	//		// Child Boxes
	//		std::vector<BoxModel*> child_boxes;
	//		child_boxes.resize(elem->children.size());

	//		auto child_node = elem->children.begin();
	//		for (uint32_t i = 0; i < child_boxes.size(); i++) {

	//			checkErrStack1(calcElementBox(*child_node, new_ancestor_width, new_ancestor_height, &box,
	//				child_boxes[i]));
	//			child_node = std::next(child_node);
	//		}

	//		// Compute positions for children
	//		div->child_positions.resize(elem->children.size());
	//		std::vector<Axis> axes;

	//		switch (div->flex_direction) {
	//		case FlexDirection::ROW: {

	//			auto calcAxisAndCrossAxisAlign = [&](uint32_t start, uint32_t end, 
	//				float used_width, float row_width, float row_height) 
	//			{
	//				float row_middle = row_height / 2;

	//				auto calcY = [&](BoxModel* child_box) -> float {

	//					switch (child_box->flex_cross_axis_align_self) {
	//					case FlexCrossAxisAlign::START:
	//						return 0;
	//					case FlexCrossAxisAlign::END:
	//						return row_height - child_box->marginbox_height;
	//					case FlexCrossAxisAlign::CENTER:
	//						return row_middle - child_box->marginbox_height / 2;
	//					case FlexCrossAxisAlign::PARENT:

	//						switch (div->flex_cross_axis_align) {
	//						case FlexCrossAxisAlign::START:
	//							return 0;
	//						case FlexCrossAxisAlign::END:
	//							return row_height - child_box->marginbox_height;
	//						case FlexCrossAxisAlign::CENTER:
	//							return row_middle - child_box->marginbox_height / 2;
	//						}
	//					}

	//					return -1;
	//				};

	//				auto packChildren = [&](float col_start_x, float step) {
	//					for (uint32_t i = start; i < end; i++) {

	//						BoxModel* child_box = child_boxes[i];
	//						glm::vec2& child_pos = div->child_positions[i];

	//						child_pos.x = col_start_x;
	//						child_pos.y = calcY(child_box);

	//						col_start_x += child_box->marginbox_width + step;
	//					}
	//				};

	//				switch (div->flex_axis_align) {
	//				case FlexAxisAlign::START: {
	//					packChildren(0, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::END: {
	//					packChildren(row_width - used_width, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::CENTER: {
	//					packChildren((row_width - used_width) / 2, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::SPACE_BETWEEN: {
	//					packChildren(0, (row_width - used_width) / ((float)child_boxes.size() - 1));
	//				}
	//				}
	//			};

	//			auto calcLinesAlign = [&](float lines_height) {

	//				auto packLines = [&](float row_start_y, float step) {

	//					uint32_t child_idx = 0;
	//					for (uint32_t row = 0; row < axes.size(); row++) {

	//						Axis& axis = axes[row];

	//						for (; child_idx < axis.idx_end; child_idx++) {

	//							BoxModel* child_box = child_boxes[child_idx];
	//							glm::vec2& child_pos = div->child_positions[child_idx];

	//							child_pos.y += row_start_y;  // relative to row -> relative to container
	//						}
	//						row_start_y += axis.cross_axis_size + step;
	//					}
	//				};

	//				switch (div->flex_lines_align) {
	//				case FlexLinesAlign::START: {
	//					packLines(0, 0);
	//					break;
	//				}

	//				case FlexLinesAlign::END: {
	//					packLines(ancestor_height - lines_height, 0);
	//					break;
	//				}
	//				case FlexLinesAlign::CENTER: {
	//					packLines((ancestor_height / 2) - (lines_height / 2), 0);
	//					break;
	//				}
	//				case FlexLinesAlign::SPACE_BETWEEN: {
	//					packLines(0, (ancestor_height - lines_height) / (axes.size() - 1));
	//				}
	//				}
	//			};

	//			switch (div->flex_wrap) {
	//			case FlexWrap::NO_WRAP: {

	//				float used_width = 0;
	//				float row_height = 0;

	//				for (uint32_t i = 0; i < child_boxes.size(); i++) {

	//					BoxModel* child_box = child_boxes[i];

	//					used_width += child_box->marginbox_width;

	//					if (child_box->marginbox_height > row_height) {
	//						row_height = child_box->marginbox_height;
	//					}
	//				}

	//				calcAxisAndCrossAxisAlign(0, (uint32_t)child_boxes.size(),
	//					used_width, ancestor_width, row_height);

	//				if (box.width.type == SizeType::FIT) {
	//					box.recalculateWidthBoxes(used_width);
	//				}
	//				if (box.height.type == SizeType::FIT) {
	//					box.recalculateHeightBoxes(row_height);
	//				}

	//				break;
	//			}

	//			case FlexWrap::WRAP: {

	//				float lines_height = 0;

	//				// calculate layout of colums
	//				float used_width = 0;
	//				float row_height = 0;			
	//				uint32_t start = 0;

	//				for (uint32_t i = 0; i < child_boxes.size(); i++) {

	//					BoxModel* child_box = child_boxes[i];

	//					if (used_width + child_box->marginbox_width > ancestor_width && i > 0) {

	//						calcAxisAndCrossAxisAlign(start, i,
	//							used_width, ancestor_width, row_height);

	//						// not used here
	//						lines_height += row_height;

	//						// reset stuff
	//						used_width = 0;
	//						row_height = 0;
	//						start = i;
	//					}

	//					used_width += child_box->marginbox_width;

	//					if (child_box->marginbox_height > row_height) {
	//						row_height = child_box->marginbox_height;
	//					}
	//				}

	//				calcLinesAlign(lines_height);

	//				if (box.height.type == SizeType::FIT) {
	//					box.recalculateHeightBoxes(lines_height);
	//				}
	//			}
	//			}
	//			break;
	//		}
	//		case FlexDirection::COLUMN: {

	//			/* children in interval [start,end) are on the same col */
	//			auto calcAxisAndCrossAxisAlign = [&](uint32_t start, uint32_t end,
	//				float used_height, float col_height, float col_width)
	//			{
	//				float col_middle = col_width / 2;

	//				auto calcX = [&](BoxModel* child_box) -> float {

	//					switch (child_box->flex_cross_axis_align_self) {
	//					case FlexCrossAxisAlign::START:
	//						return 0;
	//					case FlexCrossAxisAlign::END:
	//						return col_width - child_box->marginbox_width;
	//					case FlexCrossAxisAlign::CENTER:
	//						return col_middle - child_box->marginbox_width / 2;
	//					case FlexCrossAxisAlign::PARENT:

	//						switch (div->flex_cross_axis_align) {
	//						case FlexCrossAxisAlign::START:
	//							return 0;
	//						case FlexCrossAxisAlign::END:
	//							return col_width - child_box->marginbox_width;
	//						case FlexCrossAxisAlign::CENTER:
	//							return col_middle - child_box->marginbox_width / 2;
	//						}
	//					}
	//					return -1;
	//				};

	//				auto packChildren = [&](float col_start_y, float step) {
	//					for (uint32_t i = start; i < end; i++) {

	//						BoxModel* child_box = child_boxes[i];
	//						glm::vec2& child_pos = div->child_positions[i];

	//						child_pos.x = calcX(child_box);
	//						child_pos.y = col_start_y;

	//						col_start_y += child_box->marginbox_width + step;
	//					}
	//				};

	//				switch (div->flex_axis_align) {
	//				case FlexAxisAlign::START: {
	//					packChildren(0, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::END: {
	//					packChildren(col_height - used_height, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::CENTER: {
	//					packChildren((col_height - used_height) / 2, 0);
	//					break;
	//				}

	//				case FlexAxisAlign::SPACE_BETWEEN: {
	//					packChildren(0, (col_height - used_height) / (child_boxes.size() - 1));
	//				}
	//				}
	//			};

	//			auto calcLinesAlign = [&](float lines_width) {

	//				auto packLines = [&](float start_col_x, float step) {

	//					uint32_t child_idx = 0;
	//					for (uint32_t col = 0; col < axes.size(); col++) {

	//						Axis& axis = axes[col];

	//						for (; child_idx < axis.idx_end; child_idx++) {

	//							BoxModel* child_box = child_boxes[child_idx];
	//							glm::vec2& child_pos = div->child_positions[child_idx];

	//							child_pos.x += start_col_x;  // relative to col -> relative to container
	//						}
	//						start_col_x += axis.cross_axis_size + step;
	//					}
	//				};

	//				switch (div->flex_lines_align) {
	//				case FlexLinesAlign::START: {
	//					packLines(0, 0);
	//					break;
	//				}

	//				case FlexLinesAlign::END: {
	//					packLines(ancestor_width - lines_width, 0);
	//					break;
	//				}
	//				case FlexLinesAlign::CENTER: {
	//					packLines((ancestor_width / 2) - (lines_width / 2), 0);
	//					break;
	//				}
	//				case FlexLinesAlign::SPACE_BETWEEN: {
	//					packLines(0, (ancestor_width - lines_width) / (axes.size() - 1));
	//				}
	//				}
	//			};

	//			switch (div->flex_wrap) {
	//			case FlexWrap::NO_WRAP: {
	//				
	//				float col_width = 0;
	//				float used_height = 0;

	//				for (uint32_t i = 0; i < child_boxes.size(); i++) {

	//					BoxModel* child_box = child_boxes[i];

	//					used_height += child_box->marginbox_width;

	//					if (child_box->marginbox_width > col_width) {
	//						col_width = child_box->marginbox_width;
	//					}
	//				}

	//				calcAxisAndCrossAxisAlign(0, (uint32_t)child_boxes.size(),
	//					used_height, ancestor_height, col_width);

	//				if (box.width.type == SizeType::FIT) {
	//					box.recalculateWidthBoxes(col_width);
	//				}
	//				if (box.height.type == SizeType::FIT) {
	//					box.recalculateHeightBoxes(used_height);
	//				}
	//				break;
	//			}
	//			case FlexWrap::WRAP: {

	//				// calculate layout or rows
	//				float col_width = 0;
	//				float used_height = 0;
	//				float lines_width = 0;
	//				uint32_t start = 0;

	//				for (uint32_t i = 0; i < child_boxes.size(); i++) {

	//					BoxModel* child_box = child_boxes[i];

	//					if (used_height + child_box->marginbox_height > ancestor_height && i > 0) {

	//						calcAxisAndCrossAxisAlign(start, i,
	//							used_height, ancestor_height, col_width);

	//						// not used here
	//						lines_width += col_width;

	//						// reset stuff
	//						used_height = 0;
	//						col_width = 0;
	//						start = i;
	//					}

	//					used_height += child_box->marginbox_height;

	//					if (child_box->marginbox_width > col_width) {
	//						col_width = child_box->marginbox_width;
	//					}
	//				}

	//				calcLinesAlign(lines_width);

	//				if (box.width.type == SizeType::FIT) {
	//					box.recalculateWidthBoxes(lines_width);
	//				}
	//			}
	//			}
	//			break;
	//		}
	//		}

	//		return ErrStack();
	//	}

	//	Wrap* wrap = std::get_if<Wrap>(&elem->elem);
	//	if (wrap != nullptr) {

	//		BoxModel& box = wrap->box;

	//		float new_ancestor_width;
	//		float new_ancestor_height;
	//		box.calculate(parent_box, ancestor_width, ancestor_height,
	//			&new_ancestor_width, &new_ancestor_height);

	//		if (!elem->children.size()) {
	//			return ErrStack();
	//		}

	//		BoxModel* child_box;;
	//		checkErrStack1(calcElementBox(elem->children.front(), new_ancestor_width, new_ancestor_height, 
	//			&box, child_box));
	//		wrap->child_pos = { 0, 0 };

	//		float half_width = child_box->marginbox_width / 2;
	//		float half_height = child_box->marginbox_height / 2;

	//		switch (wrap->position) {
	//		case Position::MARGINS: {
	//			
	//			if (wrap->left_defined) {
	//				wrap->child_pos.x = calcSize(wrap->left, ancestor_width);
	//			}
	//			else if (wrap->right_defined) {
	//				wrap->child_pos.x = box.content_width - (child_box->marginbox_width + calcSize(wrap->right, ancestor_width));
	//			}

	//			if (wrap->top_defined) {
	//				wrap->child_pos.y = calcSize(wrap->top, ancestor_height);
	//			}
	//			else if (wrap->bot_defined) {
	//				wrap->child_pos.y = box.content_height - (child_box->marginbox_height + calcSize(wrap->bot, ancestor_height));
	//			}
	//			break;
	//		}

	//		case Position::CENTER: {

	//			if (wrap->left_defined) {
	//				wrap->child_pos.x = calcSize(wrap->left, ancestor_width) - half_width;
	//			}
	//			else if (wrap->right_defined) {
	//				wrap->child_pos.x = box.content_width - (half_width + calcSize(wrap->right, ancestor_width));
	//			}

	//			if (wrap->top_defined) {
	//				wrap->child_pos.y = calcSize(wrap->top, ancestor_height) - half_height;
	//			}
	//			else if (wrap->bot_defined) {
	//				wrap->child_pos.y = box.content_height - (half_height + calcSize(wrap->bot, ancestor_height));
	//			}
	//		}
	//		}
	//		return ErrStack();
	//	}

	//	Paragraph* par = std::get_if<Paragraph>(&elem->elem);
	//	if (par != nullptr) {

	//		BoxModel& box = par->box;
	//		box.calculate(parent_box, ancestor_width, ancestor_height);

	//		box.content_height = 0;
	//		box.content_width = 0;

	//		FontSize* font = findBestFitFontSize(par->font_family, par->font_style, par->font_size);

	//		float scale_unit = par->font_size / font->raster_size;
	//		float line_ascender = font->ascender * par->line_height_scale * scale_unit;
	//		float line_descender = font->descender * par->line_height_scale * scale_unit;
	//		float line_height = font->height * par->line_height_scale * scale_unit;
	//		float line_width = 0;

	//		float new_content_width = 0;

	//		glm::vec2 pen_pos = { 0, line_ascender };

	//		if (!par->wrap_text) {

	//			for (uint32_t i = 0; i < par->chars.size(); i++) {

	//				CharRaster* raster = font->findCharUnicode(par->chars[i]);

	//				float advance = raster->advance * scale_unit;
	//				float descender = (raster->height - raster->baseline_height) * scale_unit;

	//				// if new line then just change pen position
	//				if (par->chars[i] == '\n') {

	//					pen_pos.x = 0;
	//					pen_pos.y += line_height;
	//				}
	//				else {
	//					CharInstance& char_inst = raster->instances.emplace_back();
	//					char_inst.screen_pos = pen_pos;
	//					char_inst.screen_pos.y += descender;  // "ag" g needs to be positioned lower on the line
	//					char_inst.scale = scale_unit;

	//					pen_pos.x += advance;

	//					if (pen_pos.x > new_content_width) {
	//						new_content_width = pen_pos.x;
	//					}
	//				}
	//			}
	//		}
	//		else {
	//			for (uint32_t i = 0; i < par->chars.size(); i++) {

	//				CharRaster* raster = font->findCharUnicode(par->chars[i]);

	//				float advance = raster->advance * scale_unit;
	//				float descender = (raster->height - raster->baseline_height) * scale_unit;

	//				// if new line then just change pen position
	//				if (par->chars[i] == '\n') {

	//					pen_pos.x = 0;
	//					pen_pos.y += line_height;
	//				}
	//				else {
	//					if (pen_pos.x + advance > box.content_width) {

	//						pen_pos.x = 0;
	//						pen_pos.y += line_height;
	//					}

	//					CharInstance& char_inst = raster->instances.emplace_back();
	//					char_inst.screen_pos = pen_pos;
	//					char_inst.screen_pos.y += descender;  // "ag" g needs to be positioned lower on the line
	//					char_inst.scale = scale_unit;

	//					pen_pos.x += advance;

	//					if (pen_pos.x > new_content_width) {
	//						new_content_width = pen_pos.x;
	//					}
	//				}
	//			}
	//		}
	//		
	//		if (box.width.type == SizeType::FIT) {
	//			box.recalculateWidthBoxes(new_content_width);
	//		}

	//		if (box.height.type == SizeType::FIT) {
	//			box.recalculateHeightBoxes(pen_pos.y + line_descender);
	//		}

	//		return ErrStack();
	//	}

	//	return ErrStack(code_location, "");
	//}

	void UserInterface::recreateGraph(float screen_width, float screen_height)
	{
		BasicElement elem = {};
		elem.width.setAbsolute(screen_width);
		elem.height.setAbsolute(screen_height);
		elem.origin = { 0, 0 };
		elem.calculateBoxModel();

		elem.background_color = { 1, 1, 0, 1 };
		elem.border_color = { 0, 1, 1, 1 };

		// Assign to node
		this->elems.clear();
		Element& new_root = this->elems.emplace_back();
		new_root.parent = nullptr;
		new_root.elem = elem;
	}

	template<typename T>
	Element* UserInterface::addElement(Element* parent, T& new_elem)
	{
		Element* new_node = &this->elems.emplace_back();
		new_node->parent = parent;
		new_node->elem = new_elem;

		parent->children.push_back(new_node);

		return new_node;
	}
	template Element* UserInterface::addElement(Element* parent, BasicElement& new_elem);

	void UserInterface::deleteElement(Element* node, std::vector<Element*>& detached_nodes)
	{
		// parent --X--> node
		Element* parent = node->parent;
		parent->children.remove(node);

		// node <--X-- children
		detached_nodes.resize(node->children.size());

		auto child = node->children.begin();
		for (uint32_t i = 0; i < node->children.size(); i++) {
			detached_nodes[i] = *child;
			detached_nodes[i]->parent = nullptr;
			child = std::next(child);
		}

		// delete the node
		for (auto n = this->elems.begin(); n != this->elems.end(); ++n) {
			// get the address of the node
			if (&(*n) == node) {
				this->elems.erase(n);
				break;
			}
		}
	}

	void UserInterface::calcElementLayout(Element* elem, uint32_t parent_layer_idx, float ancestor_width, float ancestor_height,
		BoxModel*& r_box)
	{
		auto basic_elem = std::get_if<BasicElement>(&elem->elem);
		if (basic_elem != nullptr) {
			
			uint32_t idx = elem->parent != nullptr ? parent_layer_idx + 1 : 0;

			// Register this element to layer
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

			basic_elem->calculateBoxModel(ancestor_width, ancestor_height);

			// Child Boxes
			std::vector<BoxModel*> children_boxes(elem->children.size());
			size_t child_idx = 0;

			for (Element* child : elem->children) {

				calcElementLayout(child, idx, ancestor_width, ancestor_height,
					children_boxes[child_idx]);

				child_idx++;
			}

			// Layout
			glm::vec2 origin = basic_elem->origin;
			for (BoxModel* child_box : children_boxes) {

				child_box->origin = origin;
				origin.x += child_box->borderbox_width;
			}

			r_box = basic_elem;
			return;
		}
	}

	void UserInterface::calcGraphLayout()
	{
		BoxModel* root_box;
		calcElementLayout(&elems.front(), 0, 0, 0, root_box);
	}
}
