// Header
#include "NuiLibrary.hpp"


using namespace nui;


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

ElementZ_Index& ElementZ_Index::operator=(int32_t new_z_index)
{
	this->type = ElementZ_IndexType::SET;
	this->z_index = new_z_index;
	return *this;
}

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
