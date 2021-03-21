
// Header
#include "NuiLibrary.hpp"

// Standard
#include <unordered_set>

// GLM
#include "glm\vec3.hpp"
#include "glm\common.hpp"


using namespace nui;


Element* nui::getElementBase(StoredElement* elem)
{
	switch (elem->index()) {
	case ElementType::ROOT:
		return std::get_if<Root>(elem);
	case ElementType::TEXT:
		return std::get_if<Text>(elem);
	case ElementType::RELATIVE_WRAP:
		return std::get_if<RelativeWrap>(elem);
	}

	return nullptr;
}

struct PassedElement {
	glm::ivec2 ancestor_pos;
	int32_t ancestor_z_index;

	StoredElement* elem;
};

//float lerp3(float a, float v0, float v1, float v2)
//{
//	if (v0 < a && a < v1) {
//		return glm::mix(v0, v1, a);
//	}
//
//	return glm::mix(v1, v2, a);
//}

#pragma warning( disable : 4701)
void Window::_updateCPU()
{
	// SteadyTime& now = instance->frame_start_time;

	std::vector<StoredElement*> now_elems;
	std::vector<StoredElement*> next_elems;

	// Events
	{
		now_elems.resize(1);
		now_elems[0] = &elements.front();

		next_elems.clear();

		while (now_elems.size()) {

			for (StoredElement* now_elem : now_elems) {

				Element* elem = getElementBase(now_elem);

				int32_t top = elem->_position[1];
				int32_t bot = top + elem->_size[1];
				int32_t left = elem->_position[0];
				int32_t right = left + elem->_size[0];

				if (left < input.mouse_x && input.mouse_x < right &&
					top < input.mouse_y && input.mouse_y < bot)
				{
					elem->_emitInsideEvents();

					for (StoredElement* child : elem->_children) {
						next_elems.push_back(child);
					}
				}
				else {
					elem->_emitOutsideEvents();
				}

				elem->_events_emited = true;
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}

		// emit outside events for skipped elements
		for (StoredElement& stored_elem : elements) {
			
			Element* elem = getElementBase(&stored_elem);
			
			if (!elem->_events_emited) {
				elem->_emitOutsideEvents();
			}
		}
	}

	// Counting
	std::vector<StoredElement*> leafs;
	{
		uint32_t char_verts_count = 0;
		uint32_t char_idxs_count = 0;
		uint32_t text_inst_count = 0;

		uint32_t rect_verts_count = 0;
		uint32_t rect_idxs_count = 0;
		uint32_t rect_inst_count = 0;

		for (StoredElement& stored_elem : elements) {

			switch (stored_elem.index()) {
			case ElementType::TEXT: {

				auto text = std::get_if<Text>(&stored_elem);

				if (text->text.size()) {

					for (char c : text->text) {

						// Line Feed / New Line
						if (c != 0x000A) {
							char_verts_count += 4;
							char_idxs_count += 6;
						}
					}

					text_inst_count++;
				}

				leafs.push_back(&stored_elem);
				break;
			}

			case ElementType::RELATIVE_WRAP: {

				auto rel = std::get_if<RelativeWrap>(&stored_elem);

				if (rel->coloring != BackgroundColoring::NONE) {
					rect_verts_count += 4;
					rect_idxs_count += 6;
					rect_inst_count++;
				}

				if (!rel->_children.size()) {
					leafs.push_back(&stored_elem);
				}
				break;
			}

			case ElementType::GRID: {

				auto grid = std::get_if<Grid>(&stored_elem);

				if (grid->coloring != BackgroundColoring::NONE) {
					rect_verts_count += 4;
					rect_idxs_count += 6;
					rect_inst_count++;
				}

				if (!grid->_children.size()) {
					leafs.push_back(&stored_elem);
				}
				break;
			}
			}
		}

		char_verts.resize(char_verts_count);
		char_idxs.resize(char_idxs_count);
		text_instances.resize(text_inst_count);

		rect_verts.resize(rect_verts_count);
		rect_idxs.resize(rect_idxs_count);
	}

	CharacterAtlas& atlas = instance->char_atlas;

	// Top Down Pass for elements that depend on parents
	{
		Element* root = std::get_if<Root>(&elements.front());
		root->_size[0] = surface_width;
		root->_size[1] = surface_height;

		for (StoredElement* child : root->_children) {
			now_elems.push_back(child);
		}

		next_elems.clear();

		while (now_elems.size()) {

			for (StoredElement* now_elem : now_elems) {

				Element* elem = getElementBase(now_elem);
				elem->_calcFirstPassSize(0);
				elem->_calcFirstPassSize(1);

				for (StoredElement* child : elem->_children) {
					next_elems.push_back(child);
				}
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Down Up Pass for elements that depend on children (starts from leafs)
	{
		uint32_t char_verts_idx = 0;
		uint32_t char_idxs_idx = 0;
		uint32_t text_inst_idx = 0;

		uint32_t rect_verts_idx = 0;
		uint32_t rect_idxs_idx = 0;

		now_elems = leafs;
		next_elems.clear();

		while (now_elems.size()) {

			for (StoredElement* now_elem : now_elems) {

				switch (now_elem->index()) {

				case ElementType::ROOT: {

					auto root = std::get_if<Root>(now_elem);

					auto calc_size = [&](uint32_t axis) {

						// Calculate child positions
						for (StoredElement* child : root->_children) {

							Element* child_base = getElementBase(child);

							int32_t& _child_pos = child_base->_position[axis];

							// Child origin
							_child_pos = std::lroundf(child_base->_size[axis] * -(child_base->origin[axis] / 100.f));

							switch (child_base->relative_position[axis].type) {
							case ElementPositionType::RELATIVE: {
								_child_pos += std::lroundf(root->_size[axis] * child_base->relative_position[axis].relative_pos);
								break;
							}
							case ElementPositionType::ABSOLUTE: {
								_child_pos += child_base->relative_position[axis].absolute_pos;
								break;
							}
							}
						}
					};

					calc_size(0);
					calc_size(1);

					break;
				}

				// Text has no size param or children and can be calculated anywhere
				case ElementType::TEXT: {

					auto text = std::get_if<Text>(now_elem);

					// Drawcall
					text->_vertex_start_idx = char_verts_idx;
					text->_index_start_idx = char_idxs_idx;
					text->_instance_start_idx = text_inst_idx;

					// On demand font rasterization
					FontSize* font_size;
					atlas.ensureFontWithSize(text->font_family, text->font_style, text->font_size, font_size);

					uint32_t line_height;
					if (text->line_height) {
						line_height = text->line_height;
					}
					else {
						line_height = font_size->ascender;
					}

					glm::uvec2 pen = { 0, 0 };
					pen.y += line_height;

					text->_size[0] = 0;

					for (uint32_t unicode : text->text) {

						switch (unicode) {
							// Line Feed / New Line
						case 0x000A: {
							pen.x = 0;
							pen.y += line_height;
							break;
						}

						default: {
							Character* chara = font_size->findCharacter(unicode);

							// Ignore Characters without a visible representation like space ( )
							if (chara->zone != nullptr) {

								uint32_t bitmap_width = chara->zone->bb_pix.getWidth();
								uint32_t bitmap_height = chara->zone->bb_pix.getHeight();

								int32_t char_top = chara->bitmap_top;

								glm::uvec2 character_pos = pen;
								character_pos.x += chara->bitmap_left;
								character_pos.y += bitmap_height - char_top;

								// Top Left
								glm::ivec2 pos = character_pos;
								pos.y -= bitmap_height;

								GPU_CharacterVertex* v = &char_verts[char_verts_idx];
								v->pos = toXM(pos);
								v->uv.x = chara->zone->bb_uv.x0;
								v->uv.y = chara->zone->bb_uv.y0;

								// Top Right
								pos = character_pos;
								pos.x += bitmap_width;
								pos.y -= bitmap_height;

								v = &char_verts[char_verts_idx + 1];
								v->pos = toXM(pos);
								v->uv.x = chara->zone->bb_uv.x1;
								v->uv.y = chara->zone->bb_uv.y0;

								// Bot Right
								pos = character_pos;
								pos.x += bitmap_width;

								v = &char_verts[char_verts_idx + 2];
								v->pos = toXM(pos);
								v->uv.x = chara->zone->bb_uv.x1;
								v->uv.y = chara->zone->bb_uv.y1;

								// Bot Left
								pos = character_pos;

								v = &char_verts[char_verts_idx + 3];
								v->pos = toXM(pos);
								v->uv.x = chara->zone->bb_uv.x0;
								v->uv.y = chara->zone->bb_uv.y1;

								// Tesselation 0 to 2
								char_idxs[char_idxs_idx + 0] = char_verts_idx + 0;
								char_idxs[char_idxs_idx + 1] = char_verts_idx + 1;
								char_idxs[char_idxs_idx + 2] = char_verts_idx + 2;

								char_idxs[char_idxs_idx + 3] = char_verts_idx + 2;
								char_idxs[char_idxs_idx + 4] = char_verts_idx + 3;
								char_idxs[char_idxs_idx + 5] = char_verts_idx + 0;

								char_verts_idx += 4;
								char_idxs_idx += 6;
							}

							// Move the writing pen to the next character
							pen.x += chara->advance_X;

							if (pen.x > text->_size[0]) {
								text->_size[0] = pen.x;
							}
						}
						}
					}

					text->_size[1] = pen.y + font_size->descender;

					// Text Instance
					GPU_TextInstance& tex_inst = text_instances[text_inst_idx];
					tex_inst.color = toXM(text->color.rgba);

					text_inst_idx++;

					// Drawcall
					text->_vertex_count = char_verts_idx - text->_vertex_start_idx;
					text->_index_count = char_idxs_idx - text->_index_start_idx;

					if (text->_parent != nullptr) {
						next_elems.push_back(text->_parent);
					}
					break;
				}

				case ElementType::RELATIVE_WRAP: {

					auto rel = std::get_if<RelativeWrap>(now_elem);

					auto calc_child_positions = [&](uint32_t axis) {

						uint32_t extent = 0;

						// Calculate child positions
						for (StoredElement* child : rel->_children) {

							Element* child_base = getElementBase(child);

							int32_t& _child_pos = child_base->_position[axis];

							// Child origin
							_child_pos = std::lroundf(child_base->_size[axis] * -(child_base->origin[axis] / 100.f));

							switch (child_base->relative_position[axis].type) {
							case ElementPositionType::RELATIVE: {
								_child_pos += std::lroundf(rel->_size[axis] * child_base->relative_position[axis].relative_pos);
								break;
							}
							case ElementPositionType::ABSOLUTE: {
								_child_pos += child_base->relative_position[axis].absolute_pos;
								break;
							}
							}

							// how much does a child size extend in a certain direction
							int32_t child_extent = _child_pos + child_base->_size[axis];
							if (child_extent > (int32_t)extent) {
								extent = child_extent;
							}
						}

						switch (rel->size[axis].type) {
						case ElementSizeType::FIT: {
							rel->_size[axis] = extent;
							break;
						}
						}
					};

					calc_child_positions(0);
					calc_child_positions(1);

					rel->BackgroundElement::_generateGPU_Data(rect_verts_idx, rect_idxs_idx);

					if (rel->_parent != nullptr) {
						next_elems.push_back(rel->_parent);
					}

					break;
				}

				case ElementType::GRID: {
					auto grid = std::get_if<Grid>(now_elem);

					grid->BackgroundElement::_generateGPU_Data(rect_verts_idx, rect_idxs_idx);

					if (grid->_parent != nullptr) {
						next_elems.push_back(grid->_parent);
					}
					break;
				}
				}
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Convert local to parent positions to screen relative
	// Establish drawing order
	{
		render_stacks.clear();
		{
			std::list<StoredElement*> drawcall_stack;
			render_stacks.insert(std::make_pair(0, drawcall_stack));
		}

		std::vector<PassedElement> now_pelems;
		{
			Root* root = std::get_if<Root>(&elements.front());
			for (StoredElement* child : root->_children) {

				PassedElement& passed_child = now_pelems.emplace_back();
				passed_child.ancestor_pos = { 0, 0 };
				passed_child.ancestor_z_index = 0;
				passed_child.elem = child;
			}
		}

		std::vector<PassedElement> next_pelems;

		while (now_pelems.size()) {

			for (PassedElement& now_elem : now_pelems) {

				Element* elem;
				glm::ivec2 origin;  // screen space position, ancestor + local to parent

				switch (now_elem.elem->index()) {

				case ElementType::TEXT: {

					auto text = std::get_if<Text>(now_elem.elem);
					elem = text;

					origin = { text->_position[0], text->_position[1] };
					origin += now_elem.ancestor_pos;

					uint32_t count = text->_vertex_start_idx + text->_vertex_count;
					for (uint32_t i = text->_vertex_start_idx; i < count; i++) {

						GPU_CharacterVertex& vertex = char_verts[i];
						vertex.pos.x += origin.x;
						vertex.pos.y += origin.y;
					}

					break;
				}

				case ElementType::RELATIVE_WRAP: {

					auto rel = std::get_if<RelativeWrap>(now_elem.elem);
					elem = rel;

					origin = { rel->_position[0], rel->_position[1] };
					origin += now_elem.ancestor_pos;

					uint32_t count = rel->_drawcall.vertex_start_idx + rel->_drawcall.vertex_count;
					for (uint32_t i = rel->_drawcall.vertex_start_idx; i < count; i++) {

						GPU_RectVertex& vertex = rect_verts[i];
						vertex.pos.x += origin.x;
						vertex.pos.y += origin.y;
					}

					break;
				}

				case ElementType::GRID: {

					auto grid = std::get_if<Grid>(now_elem.elem);
					elem = grid;

					origin = { grid->_position[0], grid->_position[1] };
					origin += now_elem.ancestor_pos;

					uint32_t count = grid->_drawcall.vertex_start_idx + grid->_drawcall.vertex_count;
					for (uint32_t i = grid->_drawcall.vertex_start_idx; i < count; i++) {

						GPU_RectVertex& vertex = rect_verts[i];
						vertex.pos.x += origin.x;
						vertex.pos.y += origin.y;
					}

					break;
				}

				default:
					throw std::exception();
				}

				int32_t child_z_index;

				if (elem->z_index.type == ElementZ_IndexType::INHERIT) {

					auto& drawcall_stack = render_stacks.at(now_elem.ancestor_z_index);
					drawcall_stack.push_back(now_elem.elem);

					child_z_index = now_elem.ancestor_z_index;
				}
				else {
					auto iter = render_stacks.find(elem->z_index.z_index);
					
					// create new stack
					if (iter == render_stacks.end()) {

						std::list<StoredElement*> render_stack = {
							now_elem.elem
						};
						render_stacks.insert(std::make_pair(elem->z_index.z_index, render_stack));
					}
					// add to existing stack
					else {
						std::list<StoredElement*>& render_stack = iter->second;
						render_stack.push_back(now_elem.elem);
					}

					child_z_index = elem->z_index.z_index;
				}

				for (StoredElement* child : elem->_children) {

					PassedElement& next_elem = next_pelems.emplace_back();
					next_elem.ancestor_pos = origin;
					next_elem.ancestor_z_index = child_z_index;
					next_elem.elem = child;
				}

				// Reset for next frame
				elem->_events_emited = false;
			}

			now_pelems.swap(next_pelems);
			next_pelems.clear();
		}
	}

	// Constant buffer data
	{
		glm::ivec2 screen_size = { surface_width, surface_height };
		gpu_constants.screen_size = toXM(screen_size);
	}

	// Viewport
	{
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)surface_width;
		viewport.Height = (float)surface_height;
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1;
	}
}
#pragma warning( default : 4701)