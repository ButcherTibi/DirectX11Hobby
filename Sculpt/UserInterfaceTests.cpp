
// Header
#include "UserInterfaceTests.hpp"


using namespace std::chrono_literals;


void ui_test::text(nui::Window* win)
{
	nui::TextCreateInfo text_info;
	text_info.id = "text_id";
	text_info.text = "This text is meant to test basic text rendering";
	text_info.font_size = 42;
	text_info.line_height = (uint32_t)(42 * 1.5);
	text_info.color = nui::AnimatedProperty(nui::Color(0.f, 0.5f, 1.f), 5s);

	win->createText(text_info);
}

void ui_test::flex(nui::Window* win)
{
	nui::FlexCreateInfo flex_info = {};
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
}

void ui_test::menu(nui::Window* win)
{
	uint32_t font_size = 14;
	uint32_t vertical_padding = 5;
	uint32_t horizontal_padding = 10;
	uint32_t border_thickness = 1;

	nui::Menu* menu;
	{
		nui::MenuCreateInfo menu_info;
		menu_info.id = "menu";
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
	title_info.highlight_color = nui::Color(0.f, 0.4f, 0.4f);

	nui::MenuItemCreateInfo item_info;
	item_info.left_text.font_size = font_size;
	item_info.top_padding = vertical_padding;
	item_info.bot_padding = vertical_padding;
	item_info.left_padding = horizontal_padding;
	item_info.right_padding = horizontal_padding;
	item_info.highlight_color = nui::Color(0.f, 0.4f, 0.4f);
	item_info.arrow_width = 6;
	item_info.arrow_height = 6;
	item_info.arrow_color = nui::Color(0.f, 0.6f, 0.6f);
	item_info.arrow_highlight_color = nui::Color::white();

	nui::SubMenuCreateInfo submenu_info;
	submenu_info.background_color = nui::Color(0.f, 0.2f, 0.2f);
	submenu_info.border_thickness = border_thickness;
	submenu_info.border_color = nui::Color::white();

	for (uint32_t title_idx = 0; title_idx < 5; title_idx++) {

		title_info.left_text.text = "Title Menu " + std::to_string(title_idx);

		uint32_t title_1 = menu->createTitle(title_info);
		uint32_t title_submenu_1 = menu->createSubMenu(title_1, submenu_info);
		{
			uint32_t section_1 = menu->createSection(title_submenu_1);
			{
				for (uint32_t i = 1; i < 5; i++) {

					item_info.left_text.text = "Item " + std::to_string(i);
					menu->createItem(section_1, item_info);
				}
			}

			uint32_t section_2 = menu->createSection(title_submenu_1);
			{
				for (uint32_t i = 5; i < 11; i++) {

					item_info.left_text.text = "Item " + std::to_string(i);

					uint32_t item = menu->createItem(section_2, item_info);
					uint32_t submenu = menu->createSubMenu(item, submenu_info);
					{
						uint32_t section = menu->createSection(submenu);
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
}
