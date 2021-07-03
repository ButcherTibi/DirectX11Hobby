
// Header
#include "NuiLibrary.hpp"


using namespace nui;


//#pragma warning(4701 : disable)
void Window::_updateCPU()
{
	// Element Events
	auto emit_element_events = [this]() {
		bool emit_inside_events = true;

		for (auto i = draw_stacks.rbegin(); i != draw_stacks.rend(); ++i) {

			std::list<Element*>& elems = i->second;

			for (auto j = elems.rbegin(); j != elems.rend(); ++j) {

				Element* elem = *j;
				elem->_emitEvents(emit_inside_events);
			}
		}
	};
	emit_element_events();

	// Window Events
	{
		if (finalEvent != nullptr) {
			finalEvent(this, final_event_user_data);
		}
	}

	// Apply changes requested by elements from change log
	{
		for (auto& change : changes) {

			switch (change.index()) {
			case ElementGraphChangeType::ADD: {
				
				auto& add = std::get<AddChange>(change);

				auto& parent = add.parent;
				parent->_children.push_back(add.elem);

				auto& elem = add.elem;
				elem->_self_children = std::prev(parent->_children.end());
				break;
			}

			case ElementGraphChangeType::UPDATE: {

				auto& update = std::get<UpdateChange>(change);

				// Update Element
				switch (update.source.index()) {
				case ChangedElementType::FLEX: {

					auto* dest = std::get_if<Flex>(update.dest);
					auto& source = std::get<Flex::Change>(update.source);

					if (source.orientation.has_value()) {
						dest->orientation = source.orientation.value();
					}
					break;
				}
				case ChangedElementType::MENU: {
					auto* dest = std::get_if<Menu>(update.dest);
					auto& source = std::get<Menu::Change>(update.source);

					if (source.titles_background_color.has_value()) {
						dest->titles_background_color = source.titles_background_color.value();
					}

					if (source.select_background_color.has_value()) {
						dest->select_background_color = source.select_background_color.value();
					}

					// Items
					for (auto& item_change : source.item_changes) {
						switch (item_change.index()) {
						case MenuItemChangeType::ADD: {

							auto& added_change = std::get<Menu::Change::AddItem>(item_change);

							// establish link between parent -> child
							added_change.parent->children.push_back(added_change.item);
							break;
						}

						case MenuItemChangeType::UPDATE: {
							auto& update_change = std::get<Menu::Change::UpdateItem>(item_change);
							MenuItem* item = update_change.item;
							
							if (update_change.text.has_value()) {
								item->text = update_change.text.value();
							}

							if (update_change.callback.has_value()) {
								item->label_callback = update_change.callback.value();
							}
							break;
						}
						}
					}

					source.item_changes.clear();
					break;
				}
				}

				// Update Base Element
				Element* dest_elem = getElementBase(update.dest);
				ChangedElement& source_elem = update.source_elem;

				if (source_elem.z_index.has_value()) {
					dest_elem->z_index = source_elem.z_index.value();
				}

				if (source_elem.size.has_value()) {
					dest_elem->size = source_elem.size.value();
				}
				break;
			}

			case ElementGraphChangeType::REMOVE: {

				auto& remove = std::get<DeleteChange>(change);
				Element* elem = getElementBase(&(*remove.target));

				// unreference from parent
				if (elem->_parent != nullptr) {
					elem->_parent->_children.erase(elem->_self_children);
				}

				// unreference from children
				for (auto& children : elem->_children) {
					children->_parent = nullptr;
				}

				elements.erase(remove.target);
				break;
			}
			}
		}

		changes.clear();
	}

	// Top Down Pass for calculating size for elements that
	// have absolute size
	// have relative size dependent on computed parent size
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

				auto calc_size_for_axis = [now_elem](uint32_t axis) {
			
					auto& elem_size = now_elem->size[axis];
					auto& computed_size = now_elem->_size[axis];

					switch (elem_size.type) {
					case ElementSizeType::RELATIVE: {
						auto& _parent_size = now_elem->_parent->_size[axis];
						computed_size = std::lroundf(_parent_size * elem_size.relative_size);
						break;
					}

					case ElementSizeType::ABSOLUTE: {
						computed_size = elem_size.absolute_size;
						break;
					}

					case ElementSizeType::FIT: {
						// size cannot be calculated at this pass
						computed_size = 0;
						break;
					}
					}
				};
				calc_size_for_axis(0);
				calc_size_for_axis(1);

				for (Element* child : now_elem->_children) {
					next_elems.push_back(child);
				}
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	}

	// Gather Leafs
	std::vector<Element*> leafs;
	{
		for (StoredElement& stored_elem : elements) {

			Element* elem = getElementBase(&stored_elem);
			if (elem->_children.size() == 0) {
				leafs.push_back(elem);
			}
		}
	}

	// Down Up Pass for elements that depend on children (starts from leafs)
	auto layout_down_up = [&]() {

		std::unordered_set<Element*> now_elems;
		now_elems.reserve(leafs.size());

		for (Element* leaf : leafs) {
			now_elems.insert(leaf);
		}

		std::unordered_set<Element*> next_elems;

		while (now_elems.size()) {

			for (Element* now_elem : now_elems) {

				if (now_elem != nullptr) {

					now_elem->_calcSizeAndRelativeChildPositions();
					next_elems.insert(now_elem->_parent);
				}
			}

			now_elems.swap(next_elems);
			next_elems.clear();
		}
	};
	layout_down_up();

	// Convert local to parent positions to screen relative
	// And generate GPU Data
	{
		draw_stacks.clear();

		struct PassedElement {
			std::array<int32_t, 2> ancestor_pos;
			int32_t ancestor_z_position;
			uint32_t ancestor_z_index;
			Element* elem;
		};

		std::vector<PassedElement> now_pelems;
		{
			Root* root = std::get_if<Root>(&elements.front());

			for (Element* child : root->_children) {

				PassedElement& passed_child = now_pelems.emplace_back();
				passed_child.ancestor_pos = { 0, 0 };
				passed_child.ancestor_z_position = 0;
				passed_child.ancestor_z_index = 1;
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

				int32_t z_position;
				if (elem->z_index == 0) {
					elem->_z_index = now_elem.ancestor_z_index;
					z_position = now_elem.ancestor_z_position + 1;
				}
				else {
					elem->_z_index = elem->z_index;
					z_position = 0;
				}

				draw_stacks[elem->_z_index].push_back(elem);

				for (Element* child : elem->_children) {

					PassedElement& next_elem = next_pelems.emplace_back();
					next_elem.ancestor_pos = elem->_position;
					next_elem.ancestor_z_position = z_position;
					next_elem.ancestor_z_index = elem->_z_index;
					next_elem.elem = child;
				}
			}

			now_pelems.swap(next_pelems);
			next_pelems.clear();
		}
	}

	// Mouse Delta Trap
	if (delta_owner_elem != nullptr) {

		// Trap the Mouse position
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

		ClipCursor(&new_trap);

		switch (delta_effect) {
		case DeltaEffectType::LOOP: {
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
			break;
		}

		case DeltaEffectType::HIDDEN: {
			setMouseVisibility(false);
			break;
		}
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
