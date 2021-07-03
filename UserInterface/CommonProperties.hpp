#pragma once
// TODO: try to make this a module.ixx

// Standard
#include <chrono>


namespace nui {

	struct Color {
		glm::vec4 rgba;

	public:
		Color();
		Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha = 255);
		Color(double red, double green, double blue, double alpha = 1.0);
		Color(float red, float green, float blue, float alpha = 1.0);
		Color(int32_t hex_without_alpha);

		static Color red();
		static Color green();
		static Color blue();

		static Color black();
		static Color white();

		static Color cyan();
		static Color magenta();
		static Color yellow();

		void setRGBA_UNORM(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.0f);
	};


	// Size /////////////////////////////////////////////////////////////////////////////
#undef RELATIVE
#undef ABSOLUTE
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
		ElementSize();
		ElementSize(int32_t size_px);
		ElementSize(float percentage);

		ElementSize& operator=(int32_t size_px);
		ElementSize& operator=(float percentage);
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
		ElementPosition& operator=(int32_t size_px);
		ElementPosition& operator=(float percentage);
	};


	// Animation ////////////////////////////////////////////////////////////////////////////

	using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;

	enum class TransitionBlendFunction {
		LINEAR,
		// smooth step
		// cubic bezier
	};

	template<typename T>
	struct AnimatedProperty {
		T start;
		T end;
		SteadyTime start_time;
		SteadyTime end_time;
		TransitionBlendFunction blend_func;

		bool isAnimated();
		T calculate(SteadyTime& now);
	};


	// Background Rect /////////////////////////////////////////////////////////////////////
	struct ColorStep {
		Color color;
		float pos;

		ColorStep() = default;
		ColorStep(Color& new_color);
	};


	enum class BackgroundColoring {
		NONE,
		FLAT_FILL,
		// LINEAR_GRADIENT,
		// STRIPE
		// SMOOTHSTEP MULTI
		// QUADRATIC/CUBIC BEZIER MULTI
		RENDERING_SURFACE
	};
}