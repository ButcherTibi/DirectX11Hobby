
// Header
#include "CommonProperties.hpp"


using namespace nui;


SteadyTime nui::frame_start_time;


Color::Color() {}

Color::Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha)
{
	this->rgba.r = (float)(red / 255);
	this->rgba.g = (float)(green / 255);
	this->rgba.b = (float)(blue / 255);
	this->rgba.a = (float)(alpha / 255);
}

Color::Color(double red, double green, double blue, double alpha)
{
	this->rgba.r = (float)red;
	this->rgba.g = (float)green;
	this->rgba.b = (float)blue;
	this->rgba.a = (float)alpha;
}

Color::Color(float red, float green, float blue, float alpha)
{
	this->rgba.r = red;
	this->rgba.g = green;
	this->rgba.b = blue;
	this->rgba.a = alpha;
}

Color::Color(int32_t hex)
{
	// white = 0x00'FF'FF'FF'FF
	float red = float(hex >> 16 & 0xFF);
	float green = float(hex >> 8 & 0xFF);
	float blue = float(hex & 0xFF);

	this->rgba.r = red / 255;
	this->rgba.g = green / 255;
	this->rgba.b = blue / 255;
	this->rgba.a = 1.0f;
}

Color::Color(glm::vec4 new_rgba)
{
	this->rgba = new_rgba;
}

Color Color::red()
{
	return Color(1.0f, 0.0f, 0.0f);
}

Color Color::green()
{
	return Color(0.0f, 1.0f, 0.0f);
}

Color Color::blue()
{
	return Color(0.0f, 0.0f, 1.0f);
}

Color Color::black()
{
	return Color(0.0f, 0.0f, 0.0f);
}

Color Color::white()
{
	return Color(1.0f, 1.0f, 1.0f);
}

Color Color::cyan()
{
	return Color(0.0f, 1.0f, 1.0f);
}

Color Color::magenta()
{
	return Color(1.0f, 0.0f, 1.0f);
}

Color Color::yellow()
{
	return Color(1.0f, 1.0f, 0.0f);

}

void Color::setRGBA_UNORM(float r, float g, float b, float a)
{
	this->rgba.r = r;
	this->rgba.g = g;
	this->rgba.b = b;
	this->rgba.a = a;
}

bool Color::operator==(Color& other)
{
	return rgba == other.rgba;
}

ElementSize::ElementSize()
{
	this->type = ElementSizeType::FIT;
}

ElementSize::ElementSize(int32_t size_px)
{
	this->type = ElementSizeType::ABSOLUTE;
	this->absolute_size = (uint32_t)size_px;
}
ElementSize::ElementSize(uint32_t size_px)
{
	this->type = ElementSizeType::ABSOLUTE;
	this->absolute_size = (uint32_t)size_px;
}

ElementSize::ElementSize(float percentage)
{
	this->type = ElementSizeType::RELATIVE;
	this->relative_size = percentage / 100.f;
}

bool ElementSize::operator==(ElementSize& other)
{
	if (this->type == other.type) {

		switch (this->type) {
		case ElementSizeType::ABSOLUTE:
			return this->absolute_size == other.absolute_size;

		case ElementSizeType::RELATIVE:
			return this->relative_size == other.relative_size;

		case ElementSizeType::FIT:
			return true;
		}
	}

	return false;
}

ElementSize& ElementSize::operator=(int32_t size_px)
{
	this->type = ElementSizeType::ABSOLUTE;
	this->absolute_size = (uint32_t)size_px;
	return *this;
}

ElementSize& ElementSize::operator=(float percentage)
{
	this->type = ElementSizeType::RELATIVE;
	this->relative_size = percentage / 100.f;
	return *this;
}

ElementPosition::ElementPosition()
{
	this->type = ElementPositionType::ABSOLUTE;
	this->absolute_pos = 0;
}

ElementPosition::ElementPosition(int32_t new_absolute_pos)
{
	this->type = ElementPositionType::ABSOLUTE;
	this->absolute_pos = new_absolute_pos;
}

ElementPosition::ElementPosition(float new_relative_pos)
{
	this->type = ElementPositionType::RELATIVE;
	this->relative_pos = new_relative_pos;
}

bool ElementPosition::operator==(ElementPosition& other)
{
	if (this->type == other.type) {

		switch (this->type) {
		case ElementPositionType::ABSOLUTE:
			return this->absolute_pos == other.absolute_pos;

		case ElementPositionType::RELATIVE:
			return this->relative_pos == other.relative_pos;
		}
	}

	return false;
}

ElementPosition& ElementPosition::operator=(int32_t size_px)
{
	this->type = ElementPositionType::ABSOLUTE;
	this->absolute_pos = size_px;
	return *this;
}

ElementPosition& ElementPosition::operator=(float percentage)
{
	this->type = ElementPositionType::RELATIVE;
	this->relative_pos = percentage / 100.0f;
	return *this;
}

Z_Index::Z_Index()
{
	this->type = Z_IndexType::INHERIT;
}

TextProps::TextProps()
{
	this->text = "";
	this->font_family = "Roboto";
	this->font_style = "Regular";
	this->font_size = 14;
	this->line_height = 0xFFFF'FFFF;
	this->color = Color::white();
}
