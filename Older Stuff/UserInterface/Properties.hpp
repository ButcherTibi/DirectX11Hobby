#pragma once

// Standard
#include <string>
#include <chrono>

// GLM
#include "glm\common.hpp"
#include "glm\vec4.hpp"
#include "glm\gtx\color_space.hpp"

#undef RELATIVE
#undef ABSOLUTE


namespace nui {

	using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;

	extern SteadyTime frame_start_time;


	// Color /////////////////////////////////////////////////////////////////////////

	struct Color {
		glm::vec4 rgba;

	public:

		// Unnamed constructors //////////////////////////////////////////////////////

		Color()
		{
			rgba = {};
		}

		Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha = (uint8_t)255)
		{
			this->rgba.r = (float)(red / 255);
			this->rgba.g = (float)(green / 255);
			this->rgba.b = (float)(blue / 255);
			this->rgba.a = (float)(alpha / 255);
		}

		Color(double red, double green, double blue, double alpha = 1.0)
		{
			this->rgba.r = (float)red;
			this->rgba.g = (float)green;
			this->rgba.b = (float)blue;
			this->rgba.a = (float)alpha;
		}

		Color(float red, float green, float blue, float alpha = 1.f)
		{
			this->rgba.r = red;
			this->rgba.g = green;
			this->rgba.b = blue;
			this->rgba.a = alpha;
		}

		Color(int32_t hex)
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

		Color(glm::vec4 new_rgba)
		{
			this->rgba = new_rgba;
		}


		// Named constructors ////////////////////////////////////////////////////////////////////

		static Color hsl(float hue_degrees, float saturation, float luminance, float alpha = 1.f)
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

		static Color red()
		{
			return Color(1.0f, 0.0f, 0.0f);
		}

		static Color green()
		{
			return Color(0.0f, 1.0f, 0.0f);
		}

		static Color blue()
		{
			return Color(0.0f, 0.0f, 1.0f);
		}

		static Color black(float alpha = 1.f)
		{
			return Color(0.0f, 0.0f, 0.0f, alpha);
		}

		static Color white(float alpha = 1.f)
		{
			return Color(1.0f, 1.0f, 1.0f, alpha);
		}

		static Color transparent()
		{
			return Color(0.f, 0.f, 0.f, 0.f);
		}

		static Color cyan()
		{
			return Color(0.0f, 1.0f, 1.0f);
		}

		static Color magenta()
		{
			return Color(1.0f, 0.0f, 1.0f);
		}

		static Color yellow()
		{
			return Color(1.0f, 1.0f, 0.0f);

		}


		// Seters //////////////////////////////////////////////////////////////////////

		void setRGBA_UNORM(float r, float g, float b, float a)
		{
			this->rgba.r = r;
			this->rgba.g = g;
			this->rgba.b = b;
			this->rgba.a = a;
		}


		// Operators

		bool operator==(Color& other)
		{
			return rgba == other.rgba;
		}
	};


	// Size /////////////////////////////////////////////////////////////////////////////

	enum class ElementSizeType {
		ABSOLUTE,
		RELATIVE,
		FIT
	};

	struct ElementSize {
		ElementSizeType type;
		uint32_t absolute_size;
		float relative_size;

	public:
		ElementSize()
		{
			this->type = ElementSizeType::FIT;
		}

		ElementSize(int32_t size_px)
		{
			this->type = ElementSizeType::ABSOLUTE;
			this->absolute_size = (uint32_t)size_px;
		}
		ElementSize(uint32_t size_px)
		{
			this->type = ElementSizeType::ABSOLUTE;
			this->absolute_size = (uint32_t)size_px;
		}

		ElementSize(float percentage)
		{
			this->type = ElementSizeType::RELATIVE;
			this->relative_size = percentage / 100.f;
		}

		bool operator==(ElementSize& other)
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

		ElementSize& operator=(int32_t size_px)
		{
			this->type = ElementSizeType::ABSOLUTE;
			this->absolute_size = (uint32_t)size_px;
			return *this;
		}

		ElementSize& operator=(float percentage)
		{
			this->type = ElementSizeType::RELATIVE;
			this->relative_size = percentage / 100.f;
			return *this;
		}
	};


	// Position /////////////////////////////////////////////////////////////////////////////
	enum class ElementPositionType {
		ABSOLUTE,
		RELATIVE,
	};

	struct ElementPosition {
		ElementPositionType type;
		int32_t absolute_pos;
		float relative_pos;

	public:
		ElementPosition()
		{
			this->type = ElementPositionType::ABSOLUTE;
			this->absolute_pos = 0;
		}

		ElementPosition(int32_t new_absolute_pos)
		{
			this->type = ElementPositionType::ABSOLUTE;
			this->absolute_pos = new_absolute_pos;
		}

		ElementPosition(float new_relative_pos)
		{
			this->type = ElementPositionType::RELATIVE;
			this->relative_pos = new_relative_pos;
		}

		bool operator==(ElementPosition& other)
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

		ElementPosition& operator=(int32_t size_px)
		{
			this->type = ElementPositionType::ABSOLUTE;
			this->absolute_pos = size_px;
			return *this;
		}

		ElementPosition& operator=(float percentage)
		{
			this->type = ElementPositionType::RELATIVE;
			this->relative_pos = percentage / 100.0f;
			return *this;
		}
	};


	// Z Index ////////////////////////////////////////////////////////////////////////////
	enum class Z_IndexType {
		INHERIT,
		ABSOLUTE,
		RELATIVE
	};

	struct Z_Index {
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

	struct TextProps {
		std::string text = "";
		std::string font_family = "Roboto";
		std::string font_style = "Regular";
		uint32_t font_size = 14;
		uint32_t line_height = 0xFFFF'FFFF;
		Color color = Color::white();
	};

	struct GlyphProperties {
		std::string* font_family;
		std::string* font_style;
		uint32_t font_size;
		uint32_t line_height;

		int32_t offset_x = 0;
		int32_t offset_y = 0;
	};


	// Animated Properties ///////////////////////////////////////////////////////////////

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

	struct SimpleBorder {
		uint32_t thickness;
		AnimatedProperty<Color> color;

		SimpleBorder()
		{
			thickness = 0;
			color = AnimatedProperty<Color>();
		}
	};

	struct Padding {
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


	// Numeric Value /////////////////////////////////////////////////////////////////

	enum class NumericValueDataType {
		INT32,
		FLOAT
	};

	template<typename T>
	struct NumericValueCreateInfo {
		T soft_min;
		T soft_max;
		T hard_min;
		T hard_max;
		T initial;

		NumericValueCreateInfo()
		{
			if constexpr (std::is_same<T, int32_t>()) {
				soft_min = 0;
				soft_max = 100;
				hard_min = std::numeric_limits<int32_t>::min();
				hard_max = std::numeric_limits<int32_t>::max();;
				initial = 50;
			}
			else if constexpr (std::is_same<T, float>()) {
				soft_min = 0.f;
				soft_max = 1.f;
				hard_min = std::numeric_limits<float>::min();;
				hard_max = std::numeric_limits<float>::max();;
				initial = 0.5f;
			}
		}
	};
}