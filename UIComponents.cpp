
// Header
#include "UIComponents.h"


namespace ui {

	void CharSeq::setCharacters(std::string text, uint32_t size,
		std::string font_family, std::string font_style, glm::vec2 pos)
	{
		this->pos = pos;
		this->characters.resize(text.size());
		auto it = characters.begin();

		for (uint32_t i = 0; i < characters.size(); i++) {

			Character& uc = *it;
			uc.unicode = text[i];
			uc.size = size;
			uc.font_family = font_family;
			uc.font_style = font_style;

			it++;
		}
	}
}
