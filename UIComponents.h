#pragma once

// Standard
#include <list>

// GLM
#include <glm\vec2.hpp>
#include <glm\vec4.hpp>


namespace ui {

	struct Character {
		uint32_t unicode;
		uint32_t size;
		std::string font_family;
		std::string font_style;

		glm::vec4 color;
	};


	struct CharSeq {
		glm::vec2 pos;
		std::list<Character> characters;
		uint32_t wrap_around_at;
		float row_spacing;

		void setCharacters(std::string text, uint32_t size,
			std::string font_family, std::string font_style, glm::vec2 pos);
	};
}
