#pragma once
// TODO: try to make this a module.ixx

// Standard
#include <chrono>
#include <optional>

// GLM
#include "glm\common.hpp"


namespace nui {

	using SteadyTime = std::chrono::time_point<std::chrono::steady_clock>;


	class Element;


	// time of start of current frame
	extern SteadyTime frame_start_time;


	struct Color {
		glm::vec4 rgba;

	public:
		Color();
		Color(int32_t red, int32_t green, int32_t blue, uint8_t alpha = 255);
		Color(double red, double green, double blue, double alpha = 1.0);
		Color(float red, float green, float blue, float alpha = 1.0);
		Color(int32_t hex_without_alpha);
		Color(glm::vec4 new_rgba);

		static Color red();
		static Color green();
		static Color blue();

		static Color black();
		static Color white();

		static Color cyan();
		static Color magenta();
		static Color yellow();

		void setRGBA_UNORM(float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.0f);

		bool operator==(Color& other);
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
		ElementSize(uint32_t size_px);
		ElementSize(float percentage);

		bool operator==(ElementSize& other);

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
		ElementPosition();
		ElementPosition(int32_t absolute_pos);
		ElementPosition(float relative_pos);

		bool operator==(ElementPosition& other);

		ElementPosition& operator=(int32_t size_px);
		ElementPosition& operator=(float percentage);
	};


	// Z Index ////////////////////////////////////////////////////////////////////////////
	enum class Z_IndexType {
		INHERIT,
		ABSOLUTE,
		RELATIVE
	};

	struct Z_Index {
		Z_IndexType type;
		int32_t value;

		Z_Index();
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
		std::string text;
		std::string font_family;
		std::string font_style;
		uint32_t font_size;
		uint32_t line_height;
		Color color;

		TextProps();
	};


	// Properties /////////////////////////////////////////////////////////////////////////

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
		SteadyTime end_time;

		T calc(AnimatedProperty<T>& prop)
		{
			if (prop.duration.count() > 0) {

				// perform animation
				if (prop.value == end) {

					// past end of animation
					if (frame_start_time >= end_time) {

						start = end;
						return end;
					}
					// normal animation
					else {
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

						throw std::exception("cannot mix type");
					}
				}
				// schedule animation
				else {
					end = prop.value;

					start_time = nui::frame_start_time;
					end_time = start_time + prop.duration;

					return start;
				}
			}
			else {
				return prop.value;
			}
		}
	};


	//template<typename T>
	//struct AnimatedProperty2 {
	//	T start;  // initial property value
	//	T end;  // target destination value

	//	SteadyTime start_time;
	//	SteadyTime end_time;
	//	TransitionBlendFunction blend_func;

	//public:

	//	AnimatedProperty2<T>& operator=(T new_value)
	//	{
	//		this->start = new_value;
	//		start_time = end_time;  // stop the animation
	//		return *this;
	//	}

	//	// schedules the value to be updated and cancels the animation
	//	AnimatedProperty2<T>& operator=(T& new_value)
	//	{
	//		this->start = new_value;
	//		start_time = end_time;  // stop the animation
	//		return *this;
	//	}

	//	void setAnimation(T& target, std::chrono::milliseconds duration,
	//		TransitionBlendFunction blend_function = TransitionBlendFunction::LINEAR)
	//	{
	//		this->end = target;

	//		this->start_time = nui::frame_start_time;
	//		this->end_time = start_time + duration;
	//		this->blend_func = blend_function;
	//	}

	//	void _calc()
	//	{
	//		if (start_time == end_time) {
	//			return;
	//		}
	//		else {
	//			switch (blend_func) {
	//			case TransitionBlendFunction::LINEAR: {
	//				if (frame_start_time >= end_time) {

	//					this->start = end;
	//					end_time = start_time;  // end animation
	//				}
	//				else {
	//					float elapsed = (float)(frame_start_time - start_time).count();
	//					float total = (float)(end_time - start_time).count();

	//					if constexpr (std::is_same<T, Color>()) {
	//						this->now.rgba = glm::mix(start.rgba, end.rgba, elapsed / total);
	//					}
	//					else if constexpr (std::is_same<T, ElementPosition>()) {

	//						switch (end.type) {
	//						case ElementPositionType::ABSOLUTE: {
	//							this->now.absolute_pos = glm::mix(start.absolute_pos, end.absolute_pos, elapsed / total);
	//							break;
	//						}
	//						case ElementPositionType::RELATIVE: {
	//							this->now.relative_pos = glm::mix(start.relative_pos, end.relative_pos, elapsed / total);
	//							break;
	//						}
	//						default:
	//							throw std::exception("cannot animate element position of type FIT");
	//						}
	//					}
	//					else if constexpr (std::is_same<T, ElementSize>()) {

	//						switch (end.type) {
	//						case ElementSizeType::ABSOLUTE: {
	//							this->now.absolute_size = glm::mix(start.absolute_size, end.absolute_size, elapsed / total);
	//							break;
	//						}
	//						case ElementSizeType::RELATIVE: {
	//							this->now.relative_size = glm::mix(start.relative_size, end.relative_size, elapsed / total);
	//							break;
	//						}
	//						default:
	//							throw std::exception("cannot animate element size of type FIT");
	//						}
	//					}
	//					else {
	//						this->now = glm::mix(start, end, elapsed / total);
	//					}
	//				}
	//				break;
	//			}
	//			}
	//		}
	//	}
	//};
}