
// Header
#include "NuiLibrary.hpp"


using namespace nui;


//#pragma warning(4701 : disable)
void Window::_updateCPU()
{
	draw_stacks.clear();

	std::vector<Element*> leafs;
	{
		for (StoredElement& stored_elem : elements) {
			
			Element* elem = getElementBase(&stored_elem);
			if (elem->_children.size() == 0) {
				leafs.push_back(elem);
			}

			draw_stacks[elem->z_index];
		}
	}

	// Events
	{
		std::vector<EventsPassedElement> now_elems;
		now_elems.resize(leafs.size());

		for (uint32_t i = 0; i < leafs.size(); i++) {

			EventsPassedElement& passed = now_elems[i];

			passed.allow_inside_event = true;
			passed.elem = leafs[i];
		}
		
		std::vector<EventsPassedElement> next_elems;

		while (now_elems.size()) {

			for (EventsPassedElement passed : now_elems) {
				passed.elem->_emitEvents(passed.allow_inside_event, next_elems);
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Top Down Pass for elements that depend on parents
	{
		std::vector<Element*> now_elems;

		Element* root = std::get_if<Root>(&elements.front());
		root->_size[0] = surface_width;
		root->_size[1] = surface_height;

		for (Element* child : root->_children) {
			now_elems.push_back(child);
		}

		std::vector<Element*> next_elems;

		while (now_elems.size()) {

			for (Element* now_elem : now_elems) {

				now_elem->_calcSizeRelativeToParent();

				for (Element* child : now_elem->_children) {
					next_elems.push_back(child);
				}

				draw_stacks[now_elem->z_index].push_back(now_elem);
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Down Up Pass for elements that depend on children (starts from leafs)
	{
		std::unordered_set<Element*> now_elems;
		now_elems.reserve(leafs.size());

		for (Element* leaf : leafs) {
			now_elems.insert(leaf);
		}

		std::unordered_set<Element*> next_elems;

		while (now_elems.size()) {

			for (Element* now_elem : now_elems) {

				now_elem->_calcSizeAndRelativeChildPositions(next_elems);
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Convert local to parent positions to screen relative
	// And generate GPU Data
	{
		struct PassedElement {
			std::array<int32_t, 2> ancestor_pos;
			Element* elem;
		};

		std::vector<PassedElement> now_pelems;
		{
			Root* root = std::get_if<Root>(&elements.front());

			for (Element* child : root->_children) {

				PassedElement& passed_child = now_pelems.emplace_back();
				passed_child.ancestor_pos = { 0, 0 };
				passed_child.elem = child;
			}
		}

		std::vector<PassedElement> next_pelems;

		while (now_pelems.size()) {

			for (PassedElement& now_elem : now_pelems) {

				// convert position to screen space
				Element* elem = now_elem.elem;
				elem->_position[0] += now_elem.ancestor_pos[0];
				elem->_position[1] += now_elem.ancestor_pos[1];

				elem->_generateGPU_Data();

				for (Element* child : elem->_children) {

					PassedElement& next_elem = next_pelems.emplace_back();
					next_elem.ancestor_pos = elem->_position;
					next_elem.elem = child;
				}
			}

			now_pelems.swap(next_pelems);
			next_pelems.clear();
		}
	}

	// Mouse Delta Trap
	if (delta_owner_elem != nullptr) {

		int32_t local_top = delta_owner_elem->_position[1];
		int32_t local_bot = local_top + delta_owner_elem->_size[1];
		int32_t local_left = delta_owner_elem->_position[0];
		int32_t local_right = local_left + delta_owner_elem->_size[0];

		RECT client_rect = getClientRectangle();
		RECT new_trap;
		new_trap.top = client_rect.top + local_top;
		new_trap.bottom = client_rect.top + local_bot;
		new_trap.left = client_rect.left + local_left;
		new_trap.right = client_rect.left + local_right;

		// reapply the mouse trap if the size or position of the traping element changes
		if (new_trap.top != delta_trap_top ||
			new_trap.bottom != delta_trap_bot ||
			new_trap.left != delta_trap_left ||
			new_trap.right != delta_trap_right)
		{
			ClipCursor(&new_trap);

			delta_trap_top = new_trap.top;
			delta_trap_bot = new_trap.bottom;
			delta_trap_left = new_trap.left;
			delta_trap_right = new_trap.right;
		}

		POINT mouse_screen_pos;
		GetCursorPos(&mouse_screen_pos);

		if (mouse_screen_pos.y <= new_trap.top) {
			setLocalMousePosition(input.mouse_x, local_bot - 1);
		}
		else if (mouse_screen_pos.y >= new_trap.bottom - 1) {
			setLocalMousePosition(input.mouse_x, local_top + 1);
		}
		else if (mouse_screen_pos.x <= new_trap.left) {
			setLocalMousePosition(local_right + 1, input.mouse_y);
		}
		else if (mouse_screen_pos.x >= new_trap.right - 1) {
			setLocalMousePosition(local_left + 1, input.mouse_y);
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
//#pragma warning(4701 : default)
