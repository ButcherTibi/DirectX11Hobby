

// Header
#include "UIComponents.h"


struct Axis {
	uint32_t idx_end;
	float cross_axis_size;
};

namespace ui {

	ElemSize::ElemSize() {};

	ElemSize::ElemSize(SizeType type, float size)
	{
		this->type = type;
		this->size = size;
	}

	static float calcSize(ElemSize size, float size_ref)
	{
		if (size.type == SizeType::RELATIVE_SIZE) {
			return size_ref * size.size;
		}
		return size.size;
	}

	void BoxModel::calculate(BoxModel* parent_box, float ancestor_width, float ancestor_height,
		float* new_ancestor_width = nullptr, float* new_ancestor_height = nullptr)
	{
		// Size reference
		float size_ref_width;
		float size_ref_height;

		switch (size_reference) {
		case SizeReference::MARGIN_BOX:
			size_ref_width = parent_box->marginbox_width;
			size_ref_height = parent_box->marginbox_height;
			break;

		case SizeReference::BORDER_BOX:
			size_ref_width = parent_box->borderbox_width;
			size_ref_height = parent_box->borderbox_height;
			break;

		case SizeReference::PADDING_BOX:
			size_ref_width = parent_box->paddingbox_width;
			size_ref_height = parent_box->paddingbox_height;
			break;

		case SizeReference::CONTENT_BOX:
			size_ref_width = parent_box->content_width;
			size_ref_height = parent_box->content_height;
			break;
		}

		// Content
		switch (width.type) {
		case SizeType::RELATIVE_SIZE:
			content_width = size_ref_width * width.size;
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
			content_height = size_ref_height * height.size;
			break;

		case SizeType::ABSOLUTE_SIZE:
			content_height = height.size;
			break;

		case SizeType::FIT:
			content_height = ancestor_height;
			break;
		}

		// Padding
		padding_left_thick = calcSize(padding_left, size_ref_width);
		padding_right_thick = calcSize(padding_right, size_ref_width);
		paddingbox_width = padding_left_thick + content_width + padding_right_thick;

		padding_top_thick = calcSize(padding_top, size_ref_height);
		padding_bot_thick = calcSize(padding_bot, size_ref_height);
		paddingbox_height = padding_top_thick + content_height + padding_bot_thick;

		// Border
		border_left_thick = calcSize(border_left, size_ref_width);
		border_right_thick = calcSize(border_right, size_ref_width);
		borderbox_width = border_left_thick + paddingbox_width + border_right_thick;

		border_top_thick = calcSize(border_top, size_ref_height);
		border_bot_thick = calcSize(border_bot, size_ref_height);
		borderbox_height = border_top_thick + paddingbox_height + border_bot_thick;

		// Margins
		margin_left_thick = calcSize(margin_left, size_ref_width);
		margin_right_thick = calcSize(margin_right, size_ref_width);
		marginbox_width = margin_left_thick + borderbox_width + margin_right_thick;

		margin_top_thick = calcSize(margin_top, size_ref_height);
		margin_bot_thick = calcSize(margin_bot, size_ref_height);
		marginbox_height = margin_top_thick + borderbox_height + margin_bot_thick;

		if (new_ancestor_width != nullptr) {

			// New Ancestor Sizes
			switch (width.type) {
			case SizeType::RELATIVE_SIZE:
			case SizeType::ABSOLUTE_SIZE:
				*new_ancestor_width = content_width < ancestor_width ?
					content_width : ancestor_width;
				break;

				// Unable to have any boxes
			case SizeType::FIT:
				*new_ancestor_width = ancestor_width;
				break;
			}

			switch (height.type) {
			case SizeType::RELATIVE_SIZE:
			case SizeType::ABSOLUTE_SIZE:
				*new_ancestor_height = content_height < ancestor_height ?
					content_height : ancestor_height;
				break;

			case SizeType::FIT:
				*new_ancestor_height = ancestor_height;
				break;
			}
		}	
	}

	void BoxModel::recalculateWidthBoxes(float new_content_width)
	{
		content_width = new_content_width;

		paddingbox_width = padding_left_thick + content_width + padding_right_thick;
		borderbox_width = border_left_thick + paddingbox_width + border_right_thick;
		marginbox_width = margin_left_thick + borderbox_width + margin_right_thick;
	}

	void BoxModel::recalculateHeightBoxes(float new_content_height)
	{
		content_height = new_content_height;

		paddingbox_height = padding_top_thick + content_height + padding_bot_thick;
		borderbox_height = border_top_thick + paddingbox_height + border_bot_thick;
		marginbox_height = margin_top_thick + borderbox_height + margin_bot_thick;
	}

	ErrStack UserInterface::calcElementBox(Node* elem, float ancestor_width, float ancestor_height, BoxModel* parent_box, 
		BoxModel*& r_box)
	{
		ErrStack err;

		Div* div = std::get_if<Div>(&elem->elem);
		if (div != nullptr) {

			BoxModel& box = div->box;

			// Calculate Box
			float new_ancestor_width;
			float new_ancestor_height;
			box.calculate(parent_box, ancestor_width, ancestor_height, 
				&new_ancestor_width, &new_ancestor_height);

			if (!elem->children.size()) {
				return ErrStack();
			}

			// Child Boxes
			std::vector<BoxModel*> child_boxes;
			child_boxes.resize(elem->children.size());

			auto child_node = elem->children.begin();
			for (uint32_t i = 0; i < child_boxes.size(); i++) {

				checkErrStack1(calcElementBox(*child_node, new_ancestor_width, new_ancestor_height, &box,
					child_boxes[i]));
				child_node = std::next(child_node);
			}

			// Compute positions for children
			div->child_positions.resize(elem->children.size());
			std::vector<Axis> axes;

			switch (div->flex_direction) {
			case FlexDirection::ROW: {

				auto calcAxisAndCrossAxisAlign = [&](uint32_t start, uint32_t end, 
					float used_width, float row_width, float row_height) 
				{
					float row_middle = row_height / 2;

					auto calcY = [&](BoxModel* child_box) -> float {

						switch (child_box->flex_cross_axis_align_self) {
						case FlexCrossAxisAlign::START:
							return 0;
						case FlexCrossAxisAlign::END:
							return row_height - child_box->marginbox_height;
						case FlexCrossAxisAlign::CENTER:
							return row_middle - child_box->marginbox_height / 2;
						case FlexCrossAxisAlign::PARENT:

							switch (div->flex_cross_axis_align) {
							case FlexCrossAxisAlign::START:
								return 0;
							case FlexCrossAxisAlign::END:
								return row_height - child_box->marginbox_height;
							case FlexCrossAxisAlign::CENTER:
								return row_middle - child_box->marginbox_height / 2;
							}
						}

						return -1;
					};

					auto packChildren = [&](float col_start_x, float step) {
						for (uint32_t i = start; i < end; i++) {

							BoxModel* child_box = child_boxes[i];
							glm::vec2& child_pos = div->child_positions[i];

							child_pos.x = col_start_x;
							child_pos.y = calcY(child_box);

							col_start_x += child_box->marginbox_width + step;
						}
					};

					switch (div->flex_axis_align) {
					case FlexAxisAlign::START: {
						packChildren(0, 0);
						break;
					}

					case FlexAxisAlign::END: {
						packChildren(row_width - used_width, 0);
						break;
					}

					case FlexAxisAlign::CENTER: {
						packChildren((row_width - used_width) / 2, 0);
						break;
					}

					case FlexAxisAlign::SPACE_BETWEEN: {
						packChildren(0, (row_width - used_width) / ((float)child_boxes.size() - 1));
					}
					}
				};

				auto calcLinesAlign = [&](float lines_height) {

					auto packLines = [&](float row_start_y, float step) {

						uint32_t child_idx = 0;
						for (uint32_t row = 0; row < axes.size(); row++) {

							Axis& axis = axes[row];

							for (; child_idx < axis.idx_end; child_idx++) {

								BoxModel* child_box = child_boxes[child_idx];
								glm::vec2& child_pos = div->child_positions[child_idx];

								child_pos.y += row_start_y;  // relative to row -> relative to container
							}
							row_start_y += axis.cross_axis_size + step;
						}
					};

					switch (div->flex_lines_align) {
					case FlexLinesAlign::START: {
						packLines(0, 0);
						break;
					}

					case FlexLinesAlign::END: {
						packLines(ancestor_height - lines_height, 0);
						break;
					}
					case FlexLinesAlign::CENTER: {
						packLines((ancestor_height / 2) - (lines_height / 2), 0);
						break;
					}
					case FlexLinesAlign::SPACE_BETWEEN: {
						packLines(0, (ancestor_height - lines_height) / (axes.size() - 1));
					}
					}
				};

				switch (div->flex_wrap) {
				case FlexWrap::NO_WRAP: {

					float used_width = 0;
					float row_height = 0;

					for (uint32_t i = 0; i < child_boxes.size(); i++) {

						BoxModel* child_box = child_boxes[i];

						used_width += child_box->marginbox_width;

						if (child_box->marginbox_height > row_height) {
							row_height = child_box->marginbox_height;
						}
					}

					calcAxisAndCrossAxisAlign(0, (uint32_t)child_boxes.size(),
						used_width, ancestor_width, row_height);

					if (box.width.type == SizeType::FIT) {
						box.recalculateWidthBoxes(used_width);
					}
					if (box.height.type == SizeType::FIT) {
						box.recalculateHeightBoxes(row_height);
					}

					break;
				}

				case FlexWrap::WRAP: {

					float lines_height = 0;

					// calculate layout of colums
					float used_width = 0;
					float row_height = 0;			
					uint32_t start = 0;

					for (uint32_t i = 0; i < child_boxes.size(); i++) {

						BoxModel* child_box = child_boxes[i];

						if (used_width + child_box->marginbox_width > ancestor_width && i > 0) {

							calcAxisAndCrossAxisAlign(start, i,
								used_width, ancestor_width, row_height);

							// not used here
							lines_height += row_height;

							// reset stuff
							used_width = 0;
							row_height = 0;
							start = i;
						}

						used_width += child_box->marginbox_width;

						if (child_box->marginbox_height > row_height) {
							row_height = child_box->marginbox_height;
						}
					}

					calcLinesAlign(lines_height);

					if (box.height.type == SizeType::FIT) {
						box.recalculateHeightBoxes(lines_height);
					}
				}
				}
				break;
			}
			case FlexDirection::COLUMN: {

				/* children in interval [start,end) are on the same col */
				auto calcAxisAndCrossAxisAlign = [&](uint32_t start, uint32_t end,
					float used_height, float col_height, float col_width)
				{
					float col_middle = col_width / 2;

					auto calcX = [&](BoxModel* child_box) -> float {

						switch (child_box->flex_cross_axis_align_self) {
						case FlexCrossAxisAlign::START:
							return 0;
						case FlexCrossAxisAlign::END:
							return col_width - child_box->marginbox_width;
						case FlexCrossAxisAlign::CENTER:
							return col_middle - child_box->marginbox_width / 2;
						case FlexCrossAxisAlign::PARENT:

							switch (div->flex_cross_axis_align) {
							case FlexCrossAxisAlign::START:
								return 0;
							case FlexCrossAxisAlign::END:
								return col_width - child_box->marginbox_width;
							case FlexCrossAxisAlign::CENTER:
								return col_middle - child_box->marginbox_width / 2;
							}
						}
						return -1;
					};

					auto packChildren = [&](float col_start_y, float step) {
						for (uint32_t i = start; i < end; i++) {

							BoxModel* child_box = child_boxes[i];
							glm::vec2& child_pos = div->child_positions[i];

							child_pos.x = calcX(child_box);
							child_pos.y = col_start_y;

							col_start_y += child_box->marginbox_width + step;
						}
					};

					switch (div->flex_axis_align) {
					case FlexAxisAlign::START: {
						packChildren(0, 0);
						break;
					}

					case FlexAxisAlign::END: {
						packChildren(col_height - used_height, 0);
						break;
					}

					case FlexAxisAlign::CENTER: {
						packChildren((col_height - used_height) / 2, 0);
						break;
					}

					case FlexAxisAlign::SPACE_BETWEEN: {
						packChildren(0, (col_height - used_height) / (child_boxes.size() - 1));
					}
					}
				};

				auto calcLinesAlign = [&](float lines_width) {

					auto packLines = [&](float start_col_x, float step) {

						uint32_t child_idx = 0;
						for (uint32_t col = 0; col < axes.size(); col++) {

							Axis& axis = axes[col];

							for (; child_idx < axis.idx_end; child_idx++) {

								BoxModel* child_box = child_boxes[child_idx];
								glm::vec2& child_pos = div->child_positions[child_idx];

								child_pos.x += start_col_x;  // relative to col -> relative to container
							}
							start_col_x += axis.cross_axis_size + step;
						}
					};

					switch (div->flex_lines_align) {
					case FlexLinesAlign::START: {
						packLines(0, 0);
						break;
					}

					case FlexLinesAlign::END: {
						packLines(ancestor_width - lines_width, 0);
						break;
					}
					case FlexLinesAlign::CENTER: {
						packLines((ancestor_width / 2) - (lines_width / 2), 0);
						break;
					}
					case FlexLinesAlign::SPACE_BETWEEN: {
						packLines(0, (ancestor_width - lines_width) / (axes.size() - 1));
					}
					}
				};

				switch (div->flex_wrap) {
				case FlexWrap::NO_WRAP: {
					
					float col_width = 0;
					float used_height = 0;

					for (uint32_t i = 0; i < child_boxes.size(); i++) {

						BoxModel* child_box = child_boxes[i];

						used_height += child_box->marginbox_width;

						if (child_box->marginbox_width > col_width) {
							col_width = child_box->marginbox_width;
						}
					}

					calcAxisAndCrossAxisAlign(0, (uint32_t)child_boxes.size(),
						used_height, ancestor_height, col_width);

					if (box.width.type == SizeType::FIT) {
						box.recalculateWidthBoxes(col_width);
					}
					if (box.height.type == SizeType::FIT) {
						box.recalculateHeightBoxes(used_height);
					}
					break;
				}
				case FlexWrap::WRAP: {

					// calculate layout or rows
					float col_width = 0;
					float used_height = 0;
					float lines_width = 0;
					uint32_t start = 0;

					for (uint32_t i = 0; i < child_boxes.size(); i++) {

						BoxModel* child_box = child_boxes[i];

						if (used_height + child_box->marginbox_height > ancestor_height && i > 0) {

							calcAxisAndCrossAxisAlign(start, i,
								used_height, ancestor_height, col_width);

							// not used here
							lines_width += col_width;

							// reset stuff
							used_height = 0;
							col_width = 0;
							start = i;
						}

						used_height += child_box->marginbox_height;

						if (child_box->marginbox_width > col_width) {
							col_width = child_box->marginbox_width;
						}
					}

					calcLinesAlign(lines_width);

					if (box.width.type == SizeType::FIT) {
						box.recalculateWidthBoxes(lines_width);
					}
				}
				}
				break;
			}
			}

			return ErrStack();
		}

		Wrap* wrap = std::get_if<Wrap>(&elem->elem);
		if (wrap != nullptr) {

			BoxModel& box = wrap->box;

			float new_ancestor_width;
			float new_ancestor_height;
			box.calculate(parent_box, ancestor_width, ancestor_height,
				&new_ancestor_width, &new_ancestor_height);

			if (!elem->children.size()) {
				return ErrStack();
			}

			BoxModel* child_box;;
			checkErrStack1(calcElementBox(elem->children.front(), new_ancestor_width, new_ancestor_height, 
				&box, child_box));
			wrap->child_pos = { 0, 0 };

			float half_width = child_box->marginbox_width / 2;
			float half_height = child_box->marginbox_height / 2;

			switch (wrap->position) {
			case Position::MARGINS: {
				
				if (wrap->left_defined) {
					wrap->child_pos.x = calcSize(wrap->left, ancestor_width);
				}
				else if (wrap->right_defined) {
					wrap->child_pos.x = box.content_width - (child_box->marginbox_width + calcSize(wrap->right, ancestor_width));
				}

				if (wrap->top_defined) {
					wrap->child_pos.y = calcSize(wrap->top, ancestor_height);
				}
				else if (wrap->bot_defined) {
					wrap->child_pos.y = box.content_height - (child_box->marginbox_height + calcSize(wrap->bot, ancestor_height));
				}
				break;
			}

			case Position::CENTER: {

				if (wrap->left_defined) {
					wrap->child_pos.x = calcSize(wrap->left, ancestor_width) - half_width;
				}
				else if (wrap->right_defined) {
					wrap->child_pos.x = box.content_width - (half_width + calcSize(wrap->right, ancestor_width));
				}

				if (wrap->top_defined) {
					wrap->child_pos.y = calcSize(wrap->top, ancestor_height) - half_height;
				}
				else if (wrap->bot_defined) {
					wrap->child_pos.y = box.content_height - (half_height + calcSize(wrap->bot, ancestor_height));
				}
			}
			}
			return ErrStack();
		}

		Paragraph* par = std::get_if<Paragraph>(&elem->elem);
		if (par != nullptr) {

			BoxModel& box = par->box;
			box.calculate(parent_box, ancestor_width, ancestor_height);

			box.content_height = 0;
			box.content_width = 0;

			FontSize* font = findBestFitFontSize(par->font_family, par->font_style, par->font_size);

			float scale_unit = par->font_size / font->raster_size;
			float line_ascender = font->ascender * par->line_height_scale * scale_unit;
			float line_descender = font->descender * par->line_height_scale * scale_unit;
			float line_height = font->height * par->line_height_scale * scale_unit;
			float line_width = 0;

			float new_content_width = 0;

			glm::vec2 pen_pos = { 0, line_ascender };

			if (!par->wrap_text) {

				for (uint32_t i = 0; i < par->chars.size(); i++) {

					CharRaster* raster = font->findCharUnicode(par->chars[i]);

					float advance = raster->advance * scale_unit;
					float descender = (raster->height - raster->baseline_height) * scale_unit;

					// if new line then just change pen position
					if (par->chars[i] == '\n') {

						pen_pos.x = 0;
						pen_pos.y += line_height;
					}
					else {
						CharInstance& char_inst = raster->instances.emplace_back();
						char_inst.screen_pos = pen_pos;
						char_inst.screen_pos.y += descender;  // "ag" g needs to be positioned lower on the line
						char_inst.scale = scale_unit;

						pen_pos.x += advance;

						if (pen_pos.x > new_content_width) {
							new_content_width = pen_pos.x;
						}
					}
				}
			}
			else {
				for (uint32_t i = 0; i < par->chars.size(); i++) {

					CharRaster* raster = font->findCharUnicode(par->chars[i]);

					float advance = raster->advance * scale_unit;
					float descender = (raster->height - raster->baseline_height) * scale_unit;

					// if new line then just change pen position
					if (par->chars[i] == '\n') {

						pen_pos.x = 0;
						pen_pos.y += line_height;
					}
					else {
						if (pen_pos.x + advance > box.content_width) {

							pen_pos.x = 0;
							pen_pos.y += line_height;
						}

						CharInstance& char_inst = raster->instances.emplace_back();
						char_inst.screen_pos = pen_pos;
						char_inst.screen_pos.y += descender;  // "ag" g needs to be positioned lower on the line
						char_inst.scale = scale_unit;

						pen_pos.x += advance;

						if (pen_pos.x > new_content_width) {
							new_content_width = pen_pos.x;
						}
					}
				}
			}
			
			if (box.width.type == SizeType::FIT) {
				box.recalculateWidthBoxes(new_content_width);
			}

			if (box.height.type == SizeType::FIT) {
				box.recalculateHeightBoxes(pen_pos.y + line_descender);
			}

			return ErrStack();
		}

		return ErrStack(code_location, "");
	}

	void UserInterface::recreateGraph(float screen_width, float screen_height)
	{
		Div root_div = {};

		root_div.flex_direction = FlexDirection::ROW;
		root_div.flex_wrap = FlexWrap::NO_WRAP;
		root_div.flex_axis_align = FlexAxisAlign::START;
		root_div.flex_cross_axis_align = FlexCrossAxisAlign::START;
		root_div.flex_lines_align = FlexLinesAlign::START;

		BoxModel& box = root_div.box;
		box.size_reference = SizeReference::BORDER_BOX;

		// Content
		box.width = { SizeType::ABSOLUTE_SIZE, screen_width };
		box.height = { SizeType::ABSOLUTE_SIZE, screen_height };

		// Padding
		box.padding_left = { SizeType::ABSOLUTE_SIZE, 0 };
		box.padding_right = { SizeType::ABSOLUTE_SIZE, 0 };
		box.padding_top = { SizeType::ABSOLUTE_SIZE, 0 };
		box.padding_bot = { SizeType::ABSOLUTE_SIZE, 0 };

		// Border
		box.border_left = {SizeType::ABSOLUTE_SIZE, 0};
		box.border_right = { SizeType::ABSOLUTE_SIZE, 0 };
		box.border_top = { SizeType::ABSOLUTE_SIZE, 0 };
		box.border_bot = { SizeType::ABSOLUTE_SIZE, 0 };

		// Margins
		box.margin_left = { SizeType::ABSOLUTE_SIZE, 0 };
		box.margin_right = { SizeType::ABSOLUTE_SIZE, 0 };
		box.margin_top = { SizeType::ABSOLUTE_SIZE, 0 };
		box.margin_bot = { SizeType::ABSOLUTE_SIZE, 0 };

		// Background
		box.background_color = { 0, 0, 0, 1 };

		box.flex_cross_axis_align_self = FlexCrossAxisAlign::PARENT;
		box.opacity = 1.0f;

		box.horizontal_overflow = Overflow::LEAK;
		box.vertical_overflow = Overflow::LEAK;
		
		// Computed
		box.content_width = screen_width;
		box.content_height = screen_height;

		box.paddingbox_width = screen_width;
		box.paddingbox_height = screen_height;

		box.borderbox_width = screen_width;
		box.borderbox_height = screen_height;

		box.marginbox_width = screen_width;
		box.marginbox_height = screen_height;

		this->nodes.clear();
		Node& new_root = this->nodes.emplace_back();
		new_root.elem = root_div;
	}

	template<typename T>
	Node* UserInterface::addNode(Node* parent, T& new_elem)
	{
		Node* new_node = &this->nodes.emplace_back();
		new_node->elem = new_elem;

		parent->parent = nullptr;
		parent->children.push_back(new_node);

		return new_node;
	}

	void UserInterface::deleteNode(Node* node, std::vector<Node*>& detached_nodes)
	{
		// parent --X--> node
		Node* parent = node->parent;
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
		for (auto n = this->nodes.begin(); n != this->nodes.end(); ++n) {
			// get the address of the node
			if (&(*n) == node) {
				this->nodes.erase(n);
				break;
			}
		}
	}


}
