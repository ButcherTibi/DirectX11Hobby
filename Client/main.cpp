
// Standard
#include <cstdio>

// Standard
#include <chrono>

// Library
import UserInterface;


using namespace std::chrono_literals;


enum class Tests {
	TEXT,
	RECT,

	BUTTON,
	SLIDER,
	DROPDOWN,

	FLEX,

	MENU,
	TREELIST
};

// modify below to test a different feature
Tests current_test = Tests::DROPDOWN;


void buttonClicked(nui::Window*, nui::StoredElement2*, void*)
{
	printf("Button Clicked \n");
}

void buttonPressed(nui::Window*, nui::StoredElement2*, void*)
{
	printf("Button Pressed \n");
}

void sliderAdjusted(nui::Window*, int32_t, float value_float, void*)
{
	printf("Slider adjusted to %f \n", value_float);
}

nui::TreeList* tree_list;

int main()
{
	nui::Instance instance;
	instance.create();

	nui::Window* window;
	{
		nui::Window::CreateInfo win_info;
		win_info.width = 1024;
		win_info.height = 700;

		window = instance.createWindow(win_info);
	}

	//// Retained Mode Elements
	//switch (current_test) {
	//case Tests::TREELIST: {

	//	// Init
	//	{
	//		nui::TreeListCreateInfo info;
	//		tree_list = window->createTreeList(info);
	//	}

	//	// Populate
	//	nui::TreeListItemCreateInfo item_info;

	//	for (uint32_t i = 1; i <= 3; i++) {

	//		item_info.text.text = "Item " + std::to_string(i);
	//		auto item_1 = tree_list->createItem(item_info);

	//		for (uint32_t j = 1; j <= 3; j++) {

	//			item_info.text.text = "Item " + std::to_string(i) + " " + std::to_string(j);
	//			auto item_1_1 = tree_list->createItem(item_1, item_info);

	//			for (uint32_t k = 1; k <= 3; k++) {

	//				item_info.text.text = "Item " + std::to_string(i) + " " +
	//					std::to_string(j) + " " + std::to_string(k);
	//				auto item_1_1_1 = tree_list->createItem(item_1_1, item_info);
	//			}
	//		}
	//	}
	//	break;
	//}
	//}

	while (window->win_messages.should_close == false) {

		window->update([](nui::Window* win, void*) {

			switch (current_test) {
			case Tests::TEXT: {

				nui::Text::CreateInfo text_info;
				text_info.id = "text_id";
				text_info.text = "This text is meant to test basic text rendering";
				text_info.font_size = 42;
				text_info.line_height = (uint32_t)(42 * 1.5);
				text_info.color = nui::AnimatedProperty(nui::Color(0.f, 0.5f, 1.f), 5s);

				win->createText(text_info);
				break;
			}

			case Tests::RECT: {

				nui::RectCreateInfo rect_info;
				rect_info.id = "rect_id";
				rect_info.size[0] = 250;
				rect_info.size[1] = 250;
				rect_info.color = nui::Color::blue();

				win->createRectangle(rect_info);
				break;
			}

			case Tests::BUTTON: {

				nui::FlexCreateInfo flex_info;
				flex_info.id = "flex_id";
				flex_info.size[0] = 100.f;
				flex_info.size[1] = 100.f;
				flex_info.orientation = nui::FlexOrientation::COLUMN;
				flex_info.items_spacing = nui::FlexSpacing::CENTER;
				flex_info.lines_spacing = nui::FlexSpacing::CENTER;

				nui::Flex* flex = win->createFlex(flex_info);

				nui::Button::CreateInfo btn_info;
				btn_info.id = "button_id";
				btn_info.border.thickness = 2;
				btn_info.border.color = nui::AnimatedProperty(nui::Color::white(), 500ms);
				btn_info.padding = nui::Padding(10, 20);
				btn_info.text = "Button";
				btn_info.font_size = 24;
				btn_info.text_color = nui::AnimatedProperty(nui::Color::white(), 500ms);
				btn_info.background_color = nui::AnimatedProperty(nui::Color::red(), 500ms);
				
				// Hover
				btn_info.hover.border_color = nui::AnimatedProperty(nui::Color::black(), 500ms);
				btn_info.hover.background_color = nui::AnimatedProperty(nui::Color::green(), 500ms);
				btn_info.hover.text_color = nui::AnimatedProperty(nui::Color::black(), 500ms);

				// Press
				btn_info.press.border_color = nui::AnimatedProperty(nui::Color::cyan(), 500ms);
				btn_info.press.background_color = nui::AnimatedProperty(nui::Color::blue(), 500ms);
				btn_info.press.text_color = nui::AnimatedProperty(nui::Color::cyan(), 500ms);
				btn_info.press.on.callback = buttonPressed;

				btn_info.click.on.callback = buttonClicked;

				flex->createButton(btn_info);

				// Variations
				btn_info.id = "button_2_id";
				btn_info.size[0] = 25.f;
				flex->createButton(btn_info);

				btn_info.id = "button_3_id";
				btn_info.size[0] = nui::ElementSize();
				btn_info.size[1] = 25.f;
				flex->createButton(btn_info);

				btn_info.id = "button_4_id";
				btn_info.size[0] = 25.f;
				btn_info.size[1] = 25.f;
				flex->createButton(btn_info);
				break;
			}

			case Tests::SLIDER: {

				nui::FlexCreateInfo flex_info;
				flex_info.id = "flex_id";
				flex_info.size[0] = 100.f;
				flex_info.size[1] = 100.f;
				flex_info.orientation = nui::FlexOrientation::COLUMN;
				flex_info.items_spacing = nui::FlexSpacing::CENTER;
				flex_info.lines_spacing = nui::FlexSpacing::CENTER;

				nui::Flex* flex = win->createFlex(flex_info);

				nui::Slider::CreateInfo slider_info;
				slider_info.id = "slider_id";
				slider_info.size[0] = 75.f;
				slider_info.thumb_adjust_callback = sliderAdjusted;
				slider_info.track_background_color = nui::AnimatedProperty(nui::Color::hsl(0.f, .0f, .3f), 500ms);
				slider_info.track_fill_color = nui::AnimatedProperty(nui::Color::hsl(0.f, .0f, .5f), 500ms);
				slider_info.thumb_color = nui::AnimatedProperty(nui::Color::hsl(0.f, .0f, 1.f), 500ms);

				slider_info.hover.track_background_color = nui::AnimatedProperty(nui::Color::hsl(0.f, .5f, .3f), 500ms);
				slider_info.hover.track_fill_color = nui::AnimatedProperty(nui::Color::hsl(200.f, .5f, .5f), 500ms);
				slider_info.hover.thumb_color = nui::AnimatedProperty(nui::Color::hsl(220.f, .5f, .5f), 500ms);

				slider_info.press.track_background_color = nui::AnimatedProperty(nui::Color::hsl(0.f, 1.f, .3f), 500ms);
				slider_info.press.track_fill_color = nui::AnimatedProperty(nui::Color::hsl(200.f, 1.f, .5f), 500ms);
				slider_info.press.thumb_color = nui::AnimatedProperty(nui::Color::hsl(220.f, 1.f, .5f), 500ms);

				flex->createSlider(slider_info);

				// Size Variations
				slider_info.id = "slider_2_id";
				slider_info.size[0] = 400;
				flex->createSlider(slider_info);
				break;
			}

			case Tests::DROPDOWN: {

				nui::FlexCreateInfo flex_info;
				flex_info.id = "flex_id";
				flex_info.size[0] = 100.f;
				flex_info.size[1] = 100.f;
				flex_info.orientation = nui::FlexOrientation::ROW;
				flex_info.items_spacing = nui::FlexSpacing::CENTER;
				flex_info.lines_spacing = nui::FlexSpacing::CENTER;

				nui::Flex* flex = win->createFlex(flex_info);

				nui::Dropdown::CreateInfo drop_info;
				drop_info.id = "dropdown_id";
				drop_info.options = {
					"Option 1", 
					"Option 2 Long",
					"Option 3"
				};

				flex->createDropdown(drop_info);

				// Size Variations
				drop_info.id = "dropdown_1_id";
				drop_info.size[0] = 35.f;
				flex->createDropdown(drop_info);
				break;
			}

			case Tests::FLEX: {

				nui::FlexCreateInfo flex_info = {};
				flex_info.id = "flex_id";
				flex_info.size[0] = 100.f;
				flex_info.size[1] = 100.f;
				flex_info.items_spacing = nui::FlexSpacing::SPACE_BETWEEN;

				nui::Flex* flex = win->createFlex(flex_info);
				{
					nui::RectCreateInfo info = {};
					info.id = "rect_0";
					info.size[0] = 100;
					info.size[1] = 100;
					info.color = nui::Color::red();

					flex->createRect(info);
				}

				{
					nui::RectCreateInfo info = {};
					info.id = "rect_1";
					info.size[0] = 100;
					info.size[1] = 100;
					info.color = nui::Color::green();

					flex->createRect(info);
				}

				{
					nui::RectCreateInfo info = {};
					info.id = "rect_2";
					info.size[0] = 100;
					info.size[1] = 100;
					info.color = nui::Color::blue();

					flex->createRect(info);
				}
				break;
			}

			case Tests::MENU: {

				uint32_t font_size = 14;
				uint32_t vertical_padding = 5;
				uint32_t horizontal_padding = 10;
				uint32_t border_thickness = 1;

				nui::Menu* menu;
				{
					nui::MenuCreateInfo menu_info;
					menu_info.id = "menu_id";
					menu_info.titles_background_color = nui::Color(0.f, 0.2f, 0.2f);
					menu_info.titles_border_thickness = border_thickness;
					menu_info.titles_border_color = nui::Color::white();

					menu = win->createMenu(menu_info);
				}

				nui::MenuItemCreateInfo title_info;
				title_info.left_text.font_size = font_size;
				title_info.top_padding = vertical_padding;
				title_info.bot_padding = vertical_padding;
				title_info.left_padding = horizontal_padding;
				title_info.right_padding = horizontal_padding;
				title_info.background_hover_color = nui::Color(0.f, 0.4f, 0.4f);

				nui::MenuItemCreateInfo item_info;
				item_info.left_text.font_size = font_size;
				item_info.top_padding = vertical_padding;
				item_info.bot_padding = vertical_padding;
				item_info.left_padding = horizontal_padding;
				item_info.right_padding = horizontal_padding;
				item_info.background_hover_color = nui::Color(0.f, 0.4f, 0.4f);
				item_info.arrow_width = 6;
				item_info.arrow_height = 6;
				item_info.arrow_color = nui::Color(0.f, 0.6f, 0.6f);
				item_info.arrow_highlight_color = nui::Color::white();

				nui::MenuSectionCreateInfo section_info;

				nui::SubMenuCreateInfo submenu_info;
				submenu_info.background_color = nui::Color(0.f, 0.2f, 0.2f);
				submenu_info.border_thickness = border_thickness;
				submenu_info.border_color = nui::Color::white();

				for (uint32_t title_idx = 0; title_idx < 5; title_idx++) {

					title_info.left_text.text = "Title Menu " + std::to_string(title_idx);

					uint32_t title_1 = menu->createTitle(title_info);
					uint32_t title_submenu_1 = menu->createSubMenu(title_1, submenu_info);
					{
						uint32_t section_1 = menu->createSection(title_submenu_1, section_info);
						{
							for (uint32_t i = 1; i < 5; i++) {

								item_info.left_text.text = "Item " + std::to_string(i);
								menu->createItem(section_1, item_info);
							}
						}

						uint32_t section_2 = menu->createSection(title_submenu_1, section_info);
						{
							for (uint32_t i = 5; i < 11; i++) {

								item_info.left_text.text = "Item " + std::to_string(i);

								uint32_t item = menu->createItem(section_2, item_info);
								uint32_t submenu = menu->createSubMenu(item, submenu_info);
								{
									uint32_t section = menu->createSection(submenu, section_info);
									{
										for (uint32_t j = 1; j < 6; j++) {

											item_info.left_text.text = "Item " + std::to_string(i) + " " + std::to_string(j);
											item_info.callback = [](nui::Window*, nui::StoredElement2*, void*) {
												printf("called \n");
											};

											menu->createItem(section, item_info);
										}
									}
								}
							}
						}
					}
				}
				break;
			}

	//		case Tests::TREELIST: {
	//			nui::FlexCreateInfo flex_info;
	//			flex_info.size[0] = 100.f;
	//			flex_info.size[1] = 100.f;
	//			flex_info.items_spacing = nui::FlexSpacing::END;
	//			auto flex = win->createFlex(flex_info);

	//			nui::TreeListCreateInfo tree_info;
	//			//tree_info.size[0] = 500;
	//			tree_info.background_color = nui::Color(0.f, .05f, .05f);
	//			flex->attachTreeList(tree_info, tree_list);
	//			break;
	//		}
			}
		});
	}
}
