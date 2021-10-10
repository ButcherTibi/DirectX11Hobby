module;

// Standard
#include <string>
#include <chrono>

// GLM
#include "glm\common.hpp"
#include "glm\vec4.hpp"
#include "glm\gtx\color_space.hpp"

export module Properties;


namespace nui {

	using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;

	export SteadyTime frame_start_time;


	// Color /////////////////////////////////////////////////////////////////////////

	export struct Color {
		glm::vec4 rgba;

	public:
		Color();
		Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha = 255);
		Color(double red, double green, double blue, double alpha = 1.0);
		Color(float red, float green, float blue, float alpha = 1.0);
		Color(int32_t hex_without_alpha);
		Color(glm::vec4 new_rgba);

		static Color hsl(float hue_degrees, float saturation = 1.f, float luminance = 0.5f, float alpha = 1.f);

		// Transparent
		static Color transparent();
		static Color transparentWhite(float alpha = 0.f);

		// Basic
		static Color black();
		static Color white();
		
		static Color red();
		static Color green();
		static Color blue();

		static Color cyan();
		static Color magenta();
		static Color yellow();

		void setRGBA_UNORM(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.0f);

		bool operator==(Color& other);
	};


	// Size /////////////////////////////////////////////////////////////////////////////

#undef RELATIVE
#undef ABSOLUTE
	export enum class ElementSizeType {
		ABSOLUTE,
		RELATIVE,
		FIT
	};

	export struct ElementSize {
		ElementSizeType type;
		uint32_t absolute_size;
		float relative_size;

	public:
		ElementSize();
		ElementSize(int32_t size_px);
		ElementSize(uint32_t size_px);
		ElementSize(float percentage);

		bool operator==(ElementSize& other);

		ElementSize& operator=(int32_t size_px);
		ElementSize& operator=(float percentage);
	};


	// Position /////////////////////////////////////////////////////////////////////////////
	export enum class ElementPositionType {
		ABSOLUTE,
		RELATIVE,
	};

	export struct ElementPosition {
		ElementPositionType type;
		int32_t absolute_pos;
		float relative_pos;

	public:
		ElementPosition();
		ElementPosition(int32_t absolute_pos);
		ElementPosition(float relative_pos);

		bool operator==(ElementPosition& other);

		ElementPosition& operator=(int32_t size_px);
		ElementPosition& operator=(float percentage);
	};


	// Z Index ////////////////////////////////////////////////////////////////////////////
	export enum class Z_IndexType {
		INHERIT,
		ABSOLUTE,
		RELATIVE
	};

	export struct Z_Index {
		Z_IndexType type = Z_IndexType::INHERIT;
		int32_t value;
	};


	// Background Rect /////////////////////////////////////////////////////////////////////

	enum class BackgroundColoring {
		NONE,
		FLAT_FILL,
		// LINEAR_GRADIENT,
		// STRIPE
		// SMOOTHSTEP MULTI
		// QUADRATIC/CUBIC BEZIER MULTI
		RENDERING_SURFACE
	};


	// Text Properties ////////////////////////////////////////////////////////////////////

	export struct TextProps {
		std::string text = "";
		std::string font_family = "Roboto";
		std::string font_style = "Regular";
		uint32_t font_size = 14;
		uint32_t line_height = 0xFFFF'FFFF;
		Color color = Color::white();;
	};


	// Animated Properties ///////////////////////////////////////////////////////////////

	export
	template<typename T>
	struct AnimatedProperty {
		T value;
		std::chrono::milliseconds duration;

		AnimatedProperty()
		{
			value = T();
			duration = std::chrono::milliseconds(0);
		}

		AnimatedProperty(T new_value, std::chrono::milliseconds new_duration = std::chrono::milliseconds(0))
		{
			value = new_value;
			duration = new_duration;
		}

		AnimatedProperty& operator=(T new_value)
		{
			this->value = new_value;
			return *this;
		}
	};

	export
	template<typename T>
	struct InternalAnimatedProperty {
		T start;
		T end;
		SteadyTime start_time;
		std::chrono::milliseconds duration;

		T mix(AnimatedProperty<T>& prop)
		{
			SteadyTime end_time = start_time + duration;

			// clamp
			if (frame_start_time >= end_time) {
				return end;
			}
			else if (frame_start_time <= start_time) {
				return start;
			}

			float elapsed = (float)(frame_start_time - start_time).count();
			float total = (float)(end_time - start_time).count();
			float alpha = elapsed / total;

			if constexpr (std::is_same<T, ElementSize>()) {

				switch (prop.value.type) {
				case ElementSizeType::ABSOLUTE: {
					return glm::mix(start.absolute_size, end.absolute_size, alpha);
				}

				case ElementSizeType::RELATIVE:
					return glm::mix(start.relative_size, end.relative_size, alpha);
				}

				// FIT
				return prop.value;
			}
			else if constexpr (std::is_same<T, ElementPosition>()) {

				switch (prop.value.type) {
				case ElementPositionType::ABSOLUTE: {
					return glm::mix(start.absolute_pos, end.absolute_pos, alpha);
				}
				}

				// Relative
				return glm::mix(start.relative_pos, end.relative_pos, alpha);
			}
			else if constexpr (std::is_same<T, Color>()) {

				return glm::mix(start.rgba, end.rgba, alpha);
			}
			else if constexpr (std::is_arithmetic<T>()) {
				// float, uint32_t, int32_t
				return glm::mix(start, end, alpha);
			}
			else {
				throw std::exception("cannot mix type, type is unsupported");
			}
		}

		T calc(AnimatedProperty<T>& prop)
		{
			if (prop.duration.count() > 0) {

				if (prop.value == end && prop.duration == duration) {

					return mix(prop);
				}
				// schedule animation
				else {
					start = mix(prop);
					end = prop.value;

					start_time = nui::frame_start_time;
					duration = prop.duration;

					return start;
				}
			}
			else {
				start = prop.value;
				end = prop.value;

				start_time = nui::frame_start_time;
				duration = std::chrono::milliseconds(0);

				return prop.value;
			}
		}
	};


	// Spacing //////////////////////////////////////////////////////////////////////////

	export struct SimpleBorder {
		uint32_t thickness;
		AnimatedProperty<Color> color;

		SimpleBorder()
		{
			thickness = 0;
			color = AnimatedProperty<Color>();
		}
	};

	export struct Padding {
		uint32_t top;
		uint32_t bot;
		uint32_t left;
		uint32_t right;

		Padding()
		{
			top = 0;
			bot = 0;
			left = 0;
			right = 0;
		}

		Padding(uint32_t vertical_padding, uint32_t horizontal_padding)
		{
			top = vertical_padding;
			bot = vertical_padding;
			left = horizontal_padding;
			right = horizontal_padding;
		}

		uint32_t width()
		{
			return left + right;
		}

		uint32_t height()
		{
			return top + bot;
		}
	};


	// Interaction //////////////////////////////////////////////////////////////////////

	/*typedef (MouseEvent*)(Window* window, std::chrono::milliseconds hover_duration,
		StoredElement2* source, void* user_data);*/


	//////////////////////////////////////////////////////////////////////////////////////
	// Implementation
	//////////////////////////////////////////////////////////////////////////////////////

	Color::Color()
	{
		
	}

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

	Color Color::hsl(float hue_degrees, float saturation, float luminance, float alpha)
	{
		float chroma = (1.f - glm::abs(2.f * luminance - 1.f)) * saturation;
		float h = hue_degrees / 60.f;
		float x = chroma * (1.f - glm::abs(glm::mod(h, 2.f) - 1.f));

		float h1, s1, l1;
		if (0.f <= h && h < 1.f) {
			h1 = chroma;
			s1 = x;
			l1 = 0;
		}
		else if (1.f <= h && h < 2.f) {
			h1 = x;
			s1 = chroma;
			l1 = 0;
		}
		else if (2.f <= h && h < 3.f) {
			h1 = 0;
			s1 = chroma;
			l1 = x;
		}
		else if (3.f <= h && h < 4.f) {
			h1 = 0;
			s1 = x;
			l1 = chroma;
		}
		else if (4.f <= h && h < 5.f) {
			h1 = x;
			s1 = 0;
			l1 = chroma;
		}
		else if (5.f <= h && h < 6.f) {
			h1 = chroma;
			s1 = 0;
			l1 = x;
		}
		else {
			h1 = 0;
			s1 = 0;
			l1 = 0;
		}

		float m = luminance - (chroma / 2.f);

		return Color(h1 + m, s1 + m, l1 + m, alpha);
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

	Color Color::transparent()
	{
		return Color(0.f, 0.f, 0.f, 0.f);
	}

	Color Color::transparentWhite(float alpha)
	{
		return Color(1.f, 1.f, 1.f, alpha);
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
}