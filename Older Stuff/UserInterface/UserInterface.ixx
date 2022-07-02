module;

// DXGI
#pragma comment(lib, "DXGI.lib")

// DirectX 11
#pragma comment(lib, "D3D11.lib")


// Standard
#include <variant>
#include <array>
#include <vector>
#include <map>
#include <unordered_set>
#include <string>
#include <chrono>
#include <limits>

// @C++_BUG: Modules Bug
// error C2953: 'std::_Tree_simple_types': class template has already been defined
// 
// @C++_BUG_WORKAROUND:
//#include <map>

// DirectX 11
#include "DX11Wrapper.hpp"

// Mine
#include "CharacterAtlas.hpp"
#include "MathStuff.hpp"
#include "Input.hpp"
#include "GPU_ShaderTypes.hpp"
#include "Properties.hpp"
#include "utf_string.hpp"


// Undefine
#undef RELATIVE
#undef ABSOLUTE
#undef min
#undef max

export module UserInterface;


/*
TODO:
- mode selector
- check box
- text input
- context menu
- tree list
- popup
- image
- tooltips
- tabs
- progress bar
- progress graph

TODO Behaviour:
- own element input for better usability

TODO Design:
- shadow rendering
- border rendering
- gradient rendering
- blur rendering
- maybe add shadow DOM
*/


// NOTES TO SELF:
// - when dealing with graphs, most of the it's better to have a root as it makes vertical traversal symetric


// BAD IDEAS:
// - using instance rendering for each unicode character
//   At first a good idea, vertex and index data computed at font sizing, only write instance data at past layout
//   but mix in element transparency and find that you need to have one drawcall per character
//   times the number of text elements.
//   This sort of rendering only make sense if render whole page and not tens of small labels
// - 

// Version 2: Full support for flex type positioning, improved event handling and shortcuts
// Version 3: Simplified the design and generalized the elements, separated elements from the drawing code,
//  removed Z Index feature
// Version 3.5: Readded Z index feature, changed how events are propagated, no longer try to pack gpu_data
//  into large buffers
// Version 4: Refactored the UI from retained to immediate mode GUI
// Version 4.1: Switched to using C++ 20 Modules
// Version 4.2: Tried to switch back to headers but I realized the benefits for clients to use headers
//   (not require clients to reference library dependencies in any way)
namespace nui {

	// Forward declarations
	export class Element;
	export class Container;

	export class Root;
	export class Text;
	export class Rect;
	export class Button;
	export class Slider;
	export class Slider2;
	export class Dropdown;
	export class DirectX11_Viewport;
	export class Stack;
	export class Flex;

	export struct MenuItem;
	export struct MenuSection;
	export class Menu;

	export class TreeList;

	typedef std::variant<
		Root, Text, Rect, Button, Slider, Slider2, Dropdown,
		DirectX11_Viewport, Stack, Flex,
		Menu, TreeList
	> StoredElement;
	export struct StoredElement2;

	export class Window;
	export class Instance;


	// Render Instance Types /////////////////////////////////////////////////////////////

	struct ClipZone {
		std::array<int32_t, 2> pos;
		std::array<uint32_t, 2> size;
	};

	struct RectInstance {
		std::array<int32_t, 2> pos;
		std::array<uint32_t, 2> size;

		Color color;

		void offsetPosition(std::array<int32_t, 2> offset)
		{
			this->pos[0] += offset[0];
			this->pos[1] += offset[1];
		}
	};

	struct ArrowInstance {
		// TODO: arrow_type
		// TODO: direction

		std::array<int32_t, 2> screen_pos;
		std::array<uint32_t, 2> size;

		Color color;
	};

	struct BorderInstance {
		std::array<int32_t, 2> screen_pos;
		std::array<uint32_t, 2> size;
		uint32_t thickness;

		Color color;

		bool top = true;
		bool bot = true;
		bool left = true;
		bool right = true;
	};

	struct PositionedCharacter {
		std::array<int32_t, 2> pos;
		Character* chara;
	};

	struct TextInstance {
		std::vector<PositionedCharacter> chars;
		Color color;

		void offsetPosition(std::array<int32_t, 2> offset)
		{
			for (PositionedCharacter& character : chars) {
				character.pos[0] += offset[0];
				character.pos[1] += offset[1];
			}
		}
	};

	struct PositionedCharacters {
		// Size of the container required to fit all characters
		uint32_t width;
		uint32_t height;

		// Font with size metrics
		FontSize* font_size;

		// Positioned Characters
		std::vector<PositionedCharacter>* chars;
	};

	struct CircleInstance {
		std::array<uint32_t, 2> pos;
		uint32_t radius;

		Color color;
	};


	// Events Component /////////////////////////////////////////////////////////////////
	typedef void (*EventCallback)(Window* window, StoredElement2* source, void* user_data);
	typedef void (*WindowCallback)(Window* window, void* user_data);

	// is the mouse inside or outside
	enum class MouseEventState {
		OUTSIDE,
		INSIDE
	};

	struct Shortcut1KeyCallback {
		uint32_t key;
		EventCallback callback;
		void* user_data;
	};

	struct Shortcut2KeysCallback {
		uint32_t key_0;
		uint32_t key_1;
		EventCallback callback;
		void* user_data;
	};

	struct Shortcut3KeysCallback {
		uint32_t key_0;
		uint32_t key_1;
		uint32_t key_2;
		EventCallback callback;
		void* user_data;
	};

	// Complex event handling component
	export class EventsComponent {
	public:
		// Internal
		Window* _window;

		MouseEventState _mouse_event_state;

		EventCallback _onMouseEnter;
		void* _mouse_enter_user_data;
		SteadyTime _mouse_enter_time;  // when did the mouse entered, used to calculated hover duration

		EventCallback _onMouseHover;
		void* _mouse_hover_user_data;

		EventCallback _onMouseMove;
		void* _mouse_move_user_data;

		EventCallback _onMouseScroll;
		void* _mouse_scroll_user_data;

		EventCallback _onMouseLeave;
		void* _mouse_leave_user_data;

		std::vector<Shortcut1KeyCallback> _key_held_downs;
		std::vector<Shortcut2KeysCallback> _keys_2_held_downs;
		std::vector<Shortcut3KeysCallback> _keys_3_held_downs;

		std::vector<Shortcut1KeyCallback> _key_downs;
		std::vector<Shortcut2KeysCallback> _keys_2_downs;
		std::vector<Shortcut3KeysCallback> _keys_3_downs;

		std::vector<Shortcut1KeyCallback> _key_ups;

	public:
		void _init(Window* window);
	
		// emit events where the mouse is inside the element
		// note that it's not this class responsability to perform collision detection with the mouse
		// it is assumend to be already decided
		void _emitInsideEvents(StoredElement2* self);
	
		// emit events where the mouse is outside the element
		// for now only leave event fits this case
		void _emitOutsideEvents(StoredElement2* self);

	public:
		void setMouseEnterEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseHoverEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseMoveEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseLeaveEvent(EventCallback callback, void* user_data = nullptr);

		// call function ONCE when all specified keys are down in order
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data = nullptr);
		void setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data = nullptr);

		// call function EVERY FRAME when all specified keys are down in order
		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data = nullptr);
		void setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data = nullptr);

		// call function once when key transitiones from down to up
		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);


		// Special Effects

		// Loops the mouse around the box
		void beginMouseLoopDeltaEffect(Box2D& trap);

		// hides the mouse and on end restore mouse position to original position
		void beginMouseFixedDeltaEffect(Box2D& trap);


		// Other

		// get how long is the mouse inside component in milliseconds
		float getInsideDuration();
	};


	// Text Input Component ///////////////////////////////////////////////////

	class TextInputComponent {
	public:
		struct CreateInfo {
			// Text
			std::string font_family = "Roboto";
			std::string font_style = "Regular";
			uint32_t font_size = 14;
			uint32_t line_height = 0xFFFF'FFFF;
			Color text_color = Color::white();

			// Selection
			Color selection_background_color = Color::blue();
			Color selection_text_color = Color::white();

			// Cursor
			Color cursor_color = Color::blue();
			uint32_t cursor_thickness = 2;
		};
		CreateInfo info;

		Window* _window = nullptr;

		utf8string display_text;

		// Cursor is after specified character ex: bar| = the | is at 'r' at index 2
		// cursor starts before() and goes to last()
		utf8string_iter cursor_pos;

		// Selection
		int32_t mouse_x_start;
		int32_t mouse_x_end;
		utf8string_iter selection_start;
		uint32_t selection_length;

		// Box2D box;
		TextInstance text_instance;
		// RectInstance select_instance;
		RectInstance cursor_instance;
	public:
		void _reset();

	public:
		void init(Window* _window);

		// Text
		void set(int32_t value);
		void set(float value, uint32_t decimal_count);
		void set(utf8string& new_text);

		// Selection
		void deselect();

		// Cursor
		void setCursorAtEnd();

		// Position
		void offsetPosition(std::array<int32_t, 2> offset);


		/* The Stages of usage */

		void respondToInput();
		
		void generateGPU_Data(uint32_t& r_width, uint32_t& r_height);

		void draw();
	};


	// Element /////////////////////////////////////////////////////////////////

	struct ElementRetainedState {
		std::string id;
		bool used;

		std::array<InternalAnimatedProperty<ElementSize>, 2> size;

		std::array<InternalAnimatedProperty<float>, 2> origin;
		std::array<InternalAnimatedProperty<ElementPosition>, 2> relative_position;
		InternalAnimatedProperty<float> flex_grow;
	};

	struct ElementCreateInfo {
		std::string id;

		std::array<AnimatedProperty<ElementSize>, 2> size;
		Z_Index z_index;

		std::array<AnimatedProperty<float>, 2> origin;
		std::array<AnimatedProperty<ElementPosition>, 2> relative_position;
		AnimatedProperty<float> flex_grow;
	};

	// Base element class used by all elements
	class Element {
	public:
		Window* _window;  // pointer to parent window some little things

		std::string id;  // id that links the retained state across frames
		
		Element* _parent;
		StoredElement2* _self;
		std::vector<Element*> _children;

		// the computed position to be sent to the GPU
		std::array<int32_t, 2> _position;

		// absolute size in pixels
		// relative size as 0% to 100% of parent computed size
		// fit size is based on the size of children
		std::array<ElementSize, 2> size;
		std::array<uint32_t, 2> _size;

		// the order in which to render the element and it's descendants
		// is 0 by default meaning inherit from parent
		Z_Index z_index;

		// the computed Z index that controls in which draw stack will this element be assigned
		int32_t _z_index;

		// the origin of the element expresed as 0% to 100% from computed size
		// works the same as the origin of a mesh in a 3D app
		std::array<float, 2> origin;

		// the position relative to parent expresed in relative or absolute units
		// also works under Window/Root
		std::array<ElementPosition, 2> relative_position;

		// how much to grow an flex item
		float flex_grow;

	public:
		void _calcNowState(ElementRetainedState* prev, ElementCreateInfo& next);


		// Virtuals in order of usage

		virtual bool _isInside();

		/**
		* element may implement this to process events
		* @param r_allow_inside_events = short-circuit the event handling chain to only 
		* 	emit outside events.
		* @param r_exclusive = makes the element that set it the only event to have
		* 	inside events emited.
		* 	To stop exclusive behaviour set it to FALSE or have the element id that set it
		* 	no longer exist.
		*/
		virtual void _emitInsideEvents(bool& r_allow_inside_events, bool& r_exclusive);
		virtual void _emitOutsideEvents();

		// element may implement this if _size requires computation
		// Example:
		// if element size[0] = Fit then size of the element depends on calculating the size of children
		// if element size cannot be determined ahead of time as in the case of the Menu element
		virtual void _calcSizeAndRelativeChildPositions();

		// implemented to load data to the GPU and draw
		virtual void _draw();
	};

	struct EventCreateInfo {
		EventCallback callback = nullptr;
		void* user_data;
	};
	

	// Text ////////////////////////////////////////////////////////////////////////////////////

	class Text : public Element {
	public:
		struct CreateInfo : public ElementCreateInfo {
			std::string text = "";
			std::string font_family = "Roboto";
			std::string font_style = "Regular";
			AnimatedProperty<uint32_t> font_size = 14;
			AnimatedProperty<uint32_t> line_height = 0xFFFF'FFFF;
			AnimatedProperty<Color> color = AnimatedProperty<Color>(Color::white());
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;

			TextInstance instance;

			InternalAnimatedProperty<uint32_t> font_size;
			InternalAnimatedProperty<uint32_t> line_height;
			InternalAnimatedProperty<Color> color;
		};
		RetainedState* state;

	public:
		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};

	//// Rect ///////////////////////////////////////////////////////////////////////////

	export struct RectCreateInfo : public ElementCreateInfo {
		AnimatedProperty<Color> color;
	};


	// Button ///////////////////////////////////////////////////////////////////////////////////

	class Button : public Element {
	public:
		struct CreateInfo : public ElementCreateInfo {
			// Layout
			SimpleBorder border;
			Padding padding;

			// Text
			std::string text = "";
			std::string font_family = "Roboto";
			std::string font_style = "Regular";
			uint32_t font_size = 14;
			uint32_t line_height = 0xFFFF'FFFF;
			AnimatedProperty<Color> text_color = AnimatedProperty<Color>(Color::white());

			// Coloring
			AnimatedProperty<Color> background_color;

			struct Hover {
				AnimatedProperty<Color> border_color;
				AnimatedProperty<Color> background_color;
				AnimatedProperty<Color> text_color;
				EventCreateInfo on;
			};
			Hover hover;

			struct Press {
				AnimatedProperty<Color> border_color;
				AnimatedProperty<Color> background_color;
				AnimatedProperty<Color> text_color;
				EventCreateInfo on;
			};
			Press press;

			struct Click {
				EventCreateInfo on;
			};
			Click click;
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;

			Box2D box;  // used for event handling and background
			BorderInstance border_instance;
			RectInstance background_instance;
			TextInstance text_instance;

			// Retained State
			bool was_down;
			InternalAnimatedProperty<Color> border_color;
			InternalAnimatedProperty<Color> background_color;
			InternalAnimatedProperty<Color> text_color;
		};
		RetainedState* state;

	public:
		bool _isInside() override;
		void _emitInsideEvents(bool&,bool&) override;
		void _emitOutsideEvents() override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// Slider ////////////////////////////////////////////////////////////////////////////////////

	typedef void (*SliderCallback)(Window* window, int32_t value_int, float value_float, void* user_data);

	class Slider : public Element {
	public:
		enum class DataType {
			INT32,
			FLOAT
		};

		struct CreateInfo : public ElementCreateInfo {
			// Track
			uint32_t track_length = 100;
			uint32_t track_background_thickness = 7;
			AnimatedProperty<Color> track_background_color = nui::Color::white(.5f);
			uint32_t track_fill_thickness = 7;
			AnimatedProperty<Color> track_fill_color = nui::Color::white(.75f);
			uint32_t track_collider_additional_length = 20;
			uint32_t track_collider_additional_thickness = 5;

			// Thumb
			uint32_t thumb_diameter = 20;
			AnimatedProperty<Color> thumb_color = nui::Color::white();

			// Data
			template<typename T>
			struct ValueCreateInfo {
				T soft_min;
				T soft_max;
				T hard_min;
				T hard_max;
				T initial;

				ValueCreateInfo()
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
			std::variant<ValueCreateInfo<int32_t>, ValueCreateInfo<float>> value_info = ValueCreateInfo<float>();
			SliderCallback thumb_adjust_callback;
			SliderCallback thumb_adjust_user_data;

			// Text
			//std::string text = "";
			//std::string font_family = "Roboto";
			//std::string font_style = "Regular";
			//uint32_t font_size = 14;
			//uint32_t line_height = 0xFFFF'FFFF;
			//AnimatedProperty<Color> text_color = AnimatedProperty<Color>(Color::white());
			//AnimatedProperty<Color> text_background_color;

			// Layout
			bool is_vertical = false;
			uint32_t text_left_padding = 5;

			struct Hover {
				AnimatedProperty<Color> track_background_color = Color::white(.5f);
				AnimatedProperty<Color> track_fill_color = Color::white(.75f);

				AnimatedProperty<Color> thumb_color = Color::white();

				//AnimatedProperty<Color> text_color;
				//AnimatedProperty<Color> text_background_color;
			};
			Hover hover;

			struct Press {
				AnimatedProperty<Color> track_background_color = Color::white(.5f);
				AnimatedProperty<Color> track_fill_color = Color::white(.75f);

				AnimatedProperty<Color> thumb_color = Color::white();

				//AnimatedProperty<Color> text_color;
				//AnimatedProperty<Color> text_background_color;
			};
			Press press;
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;

			Box2D slider_box;
			float slider_fill_ratio = -1.f;

			RectInstance track_background_instance;
			RectInstance track_fill_instance;
			CircleInstance circle_instance;
			TextInstance text_instance;

			InternalAnimatedProperty<Color> track_background_color;
			InternalAnimatedProperty<Color> track_fill_color;
			InternalAnimatedProperty<Color> thumb_color;
		};
		RetainedState* state;

	public:
		bool _isInside() override;
		void _emitInsideEvents(bool&, bool&) override;
		void _emitOutsideEvents() override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};

	// Slider 2 ///////////////////////////////////////////////////////////////////////

	typedef void (*Slider2Callback)(Window* window, float value, void* user_data);

	class Slider2 : public Element {
	public:
		struct CreateInfo : public ElementCreateInfo {
			
			// Background
			AnimatedProperty<Color> background_color = Color::red();
			AnimatedProperty<Color> fill_color = Color::blue();

			struct Label {
				utf8string text;

				// Style
				std::string font_family = "Roboto";
				std::string font_style = "Regular";
				uint32_t font_size = 14;
				uint32_t line_height = 0xFFFF'FFFF;
				AnimatedProperty<Color> color = Color::white();

				// Layout
				uint32_t padding_left = 5;
			};
			Label label;

			struct Value {
				std::string font_family = "Roboto";
				std::string font_style = "Regular";
				uint32_t font_size = 14;
				uint32_t line_height = 0xFFFF'FFFF;
				AnimatedProperty<Color> color = Color::white();

				// Layout
				uint32_t padding_right = 5;

				// Data
				float soft_min = 0.f;
				float soft_max = 1.f;
				float hard_min = std::numeric_limits<float>::min();;
				float hard_max = std::numeric_limits<float>::max();;
				float initial = 0.5f;
				uint32_t decimal_places = 2;

				// Input
				uint32_t movement_threshold = 10;
				//Color input_selection_color = Color::blue();
				//uint32_t input_cursor_thickness = 2;
			};
			Value value;

			Slider2Callback callback = nullptr;
			void* user_data;

			struct Hover {
				AnimatedProperty<Color> background_color = Color::red();
				AnimatedProperty<Color> fill_color = Color::blue();

				AnimatedProperty<Color> label_color = Color::white();
				AnimatedProperty<Color> value_color = Color::white();
			};
			Hover hover;
		};

		enum class _EditMode {
			SLIDER,
			VALUE_EDIT
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;
			
			float value;

			_EditMode mode = _EditMode::SLIDER;
			int32_t mouse_start_x, mouse_start_y;
			bool moved_more = false;
			float initial_fill_ratio;
			float fill_ratio = -1.f;

			Box2D box;

			RectInstance background_instance;
			RectInstance fill_instance;
			TextInstance label_instance;
			TextInstance value_instance;
			TextInputComponent text_input;

			InternalAnimatedProperty<Color> background_color;
			InternalAnimatedProperty<Color> fill_color;
			InternalAnimatedProperty<Color> label_color;
			InternalAnimatedProperty<Color> value_color;
		};
		RetainedState* state;

	public:
		bool _isInside() override;
		void _emitInsideEvents(bool&, bool&) override;
		void _emitOutsideEvents() override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// Dropdown ///////////////////////////////////////////////////////////////////////

	typedef void (*DropdownCallback)(Window*, uint32_t option_idx, void* user_data);

	class Dropdown : public Element {
	public:
		struct CreateInfo : public ElementCreateInfo {
			// Data
			std::vector<std::string> options;
			DropdownCallback chosen_callback = nullptr;
			void* chosen_user_data;

			// Text
			std::string text = "";
			std::string font_family = "Roboto";
			std::string font_style = "Regular";
			uint32_t font_size = 14;
			uint32_t line_height = 0xFFFF'FFFF;
			AnimatedProperty<Color> text_color = Color::black();
			AnimatedProperty<Color> background_color = Color::white();

			// Arrow
			struct Arrow {
				uint32_t width;
				uint32_t height;

				AnimatedProperty<Color> color = Color::black();
			};
			Arrow arrow;

			// Layout
			uint32_t vertical_padding = 5;
			uint32_t side_padding = 5;
			uint32_t arrow_text_padding = 5;

			struct Selected {
				AnimatedProperty<Color> text_color = Color::black();
				AnimatedProperty<Color> background_color = Color::white();
				AnimatedProperty<Color> arrow_color = Color::white();
			};
			Selected selected;

			struct Hover {
				AnimatedProperty<Color> text_color = Color::black();
				AnimatedProperty<Color> background_color = Color::white();
				AnimatedProperty<Color> arrow_color = Color::white();
			};
			Hover hover;
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;

			bool is_open = false;
			uint32_t selected_index = 0;
			uint32_t hover_index = 0xFFFF'FFFF;
			ArrowInstance arrow_instance;

			std::vector<Box2D> boxes;
			std::vector<TextInstance> text_instances;
			std::vector<RectInstance> background_instances;

			InternalAnimatedProperty<Color> text_color;
			InternalAnimatedProperty<Color> background_color;

			InternalAnimatedProperty<Color> hover_text_color;
		};

		RetainedState* state;

	public:
		bool _isInside() override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// DirectX11_Viewport ////////////////////////////////////////////////////////////////////////

	export struct DirectX11_DrawEvent {
		// Resource
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* im_ctx3;

		std::array<uint32_t, 2> render_target_size;
		ID3D11RenderTargetView* render_target;

		// Drawcall
		std::array<int32_t, 2> viewport_pos;
		std::array<uint32_t, 2> viewport_size;
	};

	typedef void(*DirectX11_DrawCallback)(Window* window, StoredElement2* source, DirectX11_DrawEvent& event, void* user_data);

	class DirectX11_Viewport : public Element {
	public:
		struct CreateInfo : public ElementCreateInfo {
			DirectX11_DrawCallback callback = nullptr;
			void* user_data;
		};

		struct RetainedState : public ElementRetainedState {
			CreateInfo info;

			Box2D box;
			EventsComponent events;
		};

		RetainedState* state;

	public:
		void _emitEvents(bool& r_allow_inside_events, bool& is_exclusive);

		void _draw() override;
	};


	// Container element class for adding children to parent elements

	export enum class FlexOrientation {
		ROW,
		COLUMN
	};

	export enum class FlexSpacing {
		START,
		CENTER,
		END,
		SPACE_BETWEEN,
	};

	export struct FlexCreateInfo : public ElementCreateInfo {
		FlexOrientation orientation = FlexOrientation::ROW;
		FlexSpacing items_spacing = FlexSpacing::START;
		FlexSpacing lines_spacing = FlexSpacing::START;
	};

	export struct MenuCreateInfo : public ElementCreateInfo {
		Color menu_background_color;
	};

	export struct TreeListCreateInfo : public ElementCreateInfo {
		Color background_color;

		// Default Properties for items
		struct Item {
			Padding padding;

			Color text_hover_color;
			Color background_hover_color;

			struct Arrow {
				uint32_t left_padding;
				uint32_t right_padding;
				uint32_t width;
				uint32_t height;
				
				Color color;
				Color hover_color;
			};
			Arrow arrow;
		};
		Item item;

		// Scroll Bar
		//uint32_t scroll_bar_thickness;
		//Color scroll_bar_color;
		//Color scroll_bar_highlight_color;
		//Color scroll_bar_background_color;

		// Layout
		uint32_t indentation;
		//uint32_t vertical_item_padding;

		TreeListCreateInfo();
	};

	class Container : public Element {
	public:

		template<typename T, typename T_CreateInfo, typename T_RetainedState>
		T* createElement(T_CreateInfo& info, std::list<T_RetainedState>& states);

		void createText(Text::CreateInfo& info);
		Rect* createRect(RectCreateInfo& info);

		void createButton(Button::CreateInfo& info);
		void createSlider(Slider::CreateInfo& info);
		void createSlider2(Slider2::CreateInfo& info);
		void createDropdown(Dropdown::CreateInfo& info);

		DirectX11_Viewport* createDirectX11_Viewport(DirectX11_Viewport::CreateInfo& info);

		Flex* createFlex(FlexCreateInfo& info);

		Menu* createMenu(MenuCreateInfo& info);

	//	void attachTreeList(TreeListCreateInfo& info, TreeList* tree_list);
	};


	// Root ////////////////////////////////////////////////////////////////////////////////////

	// having a mostly unused root element is helpfull for symentric graph traversal
	class Root : public Container {
	public:
		EventsComponent _events;

		bool _isInside() override;
		void _emitInsideEvents(bool&, bool&) override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// Rectangle //////////////////////////////////////////////////////////////////////////////////

	class Rect : public Container {
	public:

		struct RetainedState : public ElementRetainedState {
			RectCreateInfo info;

			InternalAnimatedProperty<Color> color;
		};

		RetainedState* state;

	public:
		void _draw() override;
	};


	//// Background Element ///////////////////////////////////////////////////////////////////////

	////struct BackgroundElement : public Element {
	////	BackgroundColoring coloring;

	////	Color background_color;
	////	AnimatedProperty<glm::vec4> _background_color;

	////	RenderingSurfaceCallback _onRenderingSurface;
	////	void* _surface_event_user_data;

	////	RectRender _rect_render;

	////	EventsComponent _events;

	////public:
	////	void _init();
	////	void _generateGPU_Data() override;
	////	void _draw() override;

	////	void setColorTransition(Color& end_color, uint32_t duration);

	////	void setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data = nullptr);
	////};


	//// Stack ///////////////////////////////////////////////////////////////////////

	class Stack : public Container {
		// Element does not have any properties, but children do

	public:
	//	//	void _calcSizeAndRelativeChildPositions() override;
	};


	// Flex ///////////////////////////////////////////////////////////////////////////////

	struct FlexRetainedState : public ElementRetainedState {

	};

	class Flex : public Container {
	public:
		//// TODO: not implmented
		//enum class SelfAlign {
		//	START,
		//	CENTER,
		//	END,
		//	STRETCH
		//};

		FlexOrientation orientation;
		FlexSpacing items_spacing;
		FlexSpacing lines_spacing;

		// TODO: delete this
		EventsComponent _events;

	public:
		bool _isInside() override;
		void _emitInsideEvents(bool&, bool&) override;
		void _emitOutsideEvents() override;

		void _calcSizeAndRelativeChildPositions() override;

		// Events
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);

		void setMouseMoveEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);
	};


	// Menu //////////////////////////////////////////////////////////////

	// @TODO: simplify, drop the section feature have only create item exist

	export struct MenuItemCreateInfo {
		// icon
		TextProps left_text;
		// TextProps right_text;

		uint32_t top_padding = 0;
		uint32_t bot_padding = 0;
		uint32_t left_padding = 0;
		uint32_t right_padding = 0;
		uint32_t arrow_left_padding = 0;

		// Color text_highlight_color
		Color background_hover_color;

		// Arrow
		uint32_t arrow_width = 14;
		uint32_t arrow_height = 14;
		Color arrow_color;
		Color arrow_highlight_color;

		// Events
		EventCallback callback = nullptr;

		// SubMenu
		Color menu_background_color;

		Color menu_border_color;
		uint32_t menu_border_thickness = 0;
	};

	struct MenuItem {
		uint32_t parent;
		std::vector<uint32_t> child_items;

		MenuItemCreateInfo info;
		Box2D item_box;
		Box2D menu_box;

		// Render
		RectInstance _background;
		TextInstance _text;
		ArrowInstance _arrow;
	};

	struct MenuVisibleMenus {
		uint32_t menu;
		uint32_t item;
	};

	struct MenuRetainedState : public ElementRetainedState {
		// Graph from last frame
		std::vector<MenuItem> prev_items;

		// Current frame graph
		std::vector<MenuItem> items;

		std::vector<MenuVisibleMenus> visible_menus;

		// Memory cache
		std::vector<TextInstance*> text_instances;
		std::vector<ArrowInstance*> arrow_instances;
	};

	class Menu : public Element {
	public:
		MenuRetainedState* state;

	public:
		bool _isInside() override;

		void _calcSizeAndRelativeChildPositions() override;
		void _draw() override;

		uint32_t createItem(uint32_t parent_section, MenuItemCreateInfo& info);
	};


	// Tree list /////////////////////////////////////////////////////////////////

	// TODO:
	// - scroll
	// - hover coloring
	// - arrow
	// - expand
	// - add
	// - delete
	// - drag and drop
	// - rename
	// - show hide
	// - select
	// - deselect
	// - multi select
	// - multi deselect
	// - select children
	// - deselect children
	// - move children
	// - delete children
	// - search
	// - background color

	export struct TreeListItemHandle {
		uint32_t item_idx;

		TreeListItemHandle() = default;
		TreeListItemHandle(uint32_t new_item_idx);
	};

	export struct TreeListItemCreateInfo {
		// Label
		TextProps text;

		// icon

		// selected callback
		// deselected callback
		// renamed callback
		// moved callback
		// deleted callback

		// icon callbacks
	};

	class TreeListItem {
	public:
		TreeList* treelist;

		uint32_t parent;
		uint32_t self_idx;
		std::vector<uint32_t> children;

		bool expanded;
		TreeListItemCreateInfo info;

		// Internal
		bool _traversed;
		std::array<uint32_t, 2> _label_size;
		TextInstance _text;
	};

	class TreeList : public Element {
	public:
	//	std::vector<TreeListItem> items;

	//	std::vector<TextInstance*> _text_instances;
	//	std::array<uint32_t, 2> required_size;
	//	// std::array<float, 2> scroll_factor;

	//	ElementRetainedState base_elem_state;
	//	TreeListCreateInfo info;

	//public:
	//	//void _emitEvents(bool& allow_inside_events) override;
	//	void _calcSizeAndRelativeChildPositions() override;
	//	void _draw() override;

	//	TreeListItemHandle createItem(TreeListItemCreateInfo& info);
	//	TreeListItemHandle createItem(TreeListItemHandle handle, TreeListItemCreateInfo& info);
	};


	////////////////////////////////////////////////////////////////////////////////

	struct StoredElement2 {
		StoredElement specific_elem;  // where elements are actually stored in memory
		Element* base_elem;  // for easy access

		template<typename T>
		T* get()
		{
			return std::get_if<T>(&specific_elem);
		}
	};

	struct PassedElement {
		std::array<int32_t, 2> ancestor_pos;
		int32_t ancestor_z_index;
		Element* elem;
	};

	class Window {
	public:
		struct CreateInfo {
			uint16_t width;
			uint16_t height;
		};

		struct Messages {
			bool should_close;
			bool is_minimized;
		};

		Instance* instance;  // parent instance to which this window belongs

		WNDCLASSW window_class;  // Win32 Window class used on creation
		HWND hwnd;  // handle to the window

		// Timing
		SteadyTime frame_used_time;  // the last's frame time of usage
		float delta_time;  // the total duration of the last frame
		uint32_t min_frame_duration_ms;  // the minimum amount of time a frame must last (60 fps limit)

		// Input data updated by WinProc
		Input input;

		// Window Size
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t surface_width;  // width of the renderable surface
		std::uint32_t surface_height;  // height of the renderable surface

		Messages win_messages;

		// Elements
		std::list<StoredElement2> elements;  // where the immeadiate mode elements are stored

		std::list<Text::RetainedState> text_prevs;
		std::list<Rect::RetainedState> rect_prevs;
		std::list<Button::RetainedState> button_prevs;
		std::list<Slider::RetainedState> slider_prevs;
		std::list<Slider2::RetainedState> slider2_prevs;
		std::list<Dropdown::RetainedState> dropdown_prevs;
		std::list<DirectX11_Viewport::RetainedState> dx11_viewport_prevs;
		std::list<FlexRetainedState> flex_prevs;
		std::list<MenuRetainedState> menu_prevs;

		std::list<StoredElement2> retained_elements;  // where the retained mode elements are stored
		Root* root;

		// the order in which to draw the elements on screen
		// also used for occlusion in emiting events
		std::map<uint32_t, std::list<Element*>> draw_stacks;

		struct MouseDeltaEffect {
			enum class Type {
				LOOP,
				HIDDEN
			};
			Type type;

			Box2D trap;  // mouse trap where mouse will be contained

			// initial mouse coordinates at the start of the delta effect (only for HIDDEN)
			uint32_t begin_mouse_x;
			uint32_t begin_mouse_y;
		};
		MouseDeltaEffect mouse_delta_effect;

		// Events
		std::string exclusive_event_element_id;

		void* final_event_user_data;
		WindowCallback finalEvent;

		// DirectX 11
		ComPtr<IDXGISwapChain1> swapchain1;
		ComPtr<ID3D11Texture2D> present_img;
		ComPtr<ID3D11RenderTargetView> present_rtv;

		dx11::ConstantBuffer cbuff;

		// Viewport
		D3D11_VIEWPORT viewport;

	public:
		//void _applyFileStyles();

		// Memory Cache
		std::vector<Element*> _downward_now_elems;
		std::vector<Element*> _downward_next_elems;
		std::vector<Element*> _leafs;
		std::unordered_set<Element*> _upward_now_elems;
		std::unordered_set<Element*> _upward_next_elems;
		std::vector<PassedElement> _now_pelems;
		std::vector<PassedElement> _next_pelems;

		void _render();

	public:
		// Immediate Mode Elements

		void createText(Text::CreateInfo& info);
		Rect* createRectangle(RectCreateInfo& info);
		void createButton(Button::CreateInfo& info);
		void createSlider(Slider::CreateInfo& info);
		void createDropdown(Dropdown::CreateInfo& info);
		Flex* createFlex(FlexCreateInfo& info);
		Menu* createMenu(MenuCreateInfo& info);


	//	// Retained Mode Elements

	//	TreeList* createTreeList(TreeListCreateInfo& info);


		// Update
		void update(WindowCallback callback);


	//	// Events

	//	// event to be executed before all element events are executed 
	//	// void setStartEvent(WindowCallback callback, void* user_data = nullptr);

	//	// event to be executed after all element events are executed
	//	void setEndEvent(WindowCallback callback, void* user_data = nullptr);

	//	void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void endMouseDeltaEffect();

		// Window
	
		// get rect of the actual rendered surface excludes border shadows and top bar
		RECT getClientRectangle();
	
		// Mouse
		bool setLocalMousePosition(uint32_t x, uint32_t y);
		bool untrapMousePosition();
		void setMouseVisibility(bool is_visible);
	};


	std::list<Window*> _created_windows;


	class Instance {
	public:
		HINSTANCE hinstance;
		HCURSOR arrow_hcursor;

		// DirectX 11 Context
		ComPtr<IDXGIFactory2> factory2;
		ComPtr<IDXGIFactory7> factory7;
		ComPtr<IDXGIAdapter> adapter;
		ComPtr<ID3D11Device> dev1;
		ComPtr<ID3D11Device5> dev5;
		ComPtr<ID3D11DeviceContext> im_ctx1;
		ComPtr<ID3D11DeviceContext3> im_ctx3;

		// Simple Vertex Shader
		ComPtr<ID3D11VertexShader> simple_vs;

		// Rasterizer State
		ComPtr<ID3D11RasterizerState> solid_back_rs;

		// Blend State
		ComPtr<ID3D11BlendState> blend_state;

		// Mini-Renderers
		struct TextRender {
			std::vector<GPU_CharacterVertex> verts;
			dx11::ArrayBuffer<GPU_CharacterVertex> vbuff;

			std::vector<uint32_t> indexes;
			dx11::ArrayBuffer<uint32_t> idxbuff;

			std::vector<GPU_TextInstance> instances;
			dx11::ArrayBuffer<GPU_TextInstance> instances_buff;

			// Character Atlas
			CharacterAtlas char_atlas;
			dx11::Texture char_atlas_tex;
			ComPtr<ID3D11SamplerState> char_atlas_sampler;

			// Shaders
			ComPtr<ID3D11VertexShader> char_vs;
			ComPtr<ID3D11PixelShader> char_ps;
		};
		TextRender text_renderer;

		struct RectRender {
			std::vector<GPU_SimpleVertex> verts;
			dx11::ArrayBuffer<GPU_SimpleVertex> vbuff;

			std::vector<uint32_t> indexes;
			dx11::ArrayBuffer<uint32_t> idxbuff;

			std::vector<GPU_RectInstance> instances;
			dx11::ArrayBuffer<GPU_RectInstance> instances_buff;

			// Shaders
			ComPtr<ID3D11PixelShader> rect_ps;
		};
		RectRender rect_renderer;

		struct CircleRender {
			std::vector<GPU_SimpleVertex> verts;
			dx11::ArrayBuffer<GPU_SimpleVertex> vbuff;

			std::vector<uint32_t> indexes;
			dx11::ArrayBuffer<uint32_t> idxbuff;

			std::vector<GPU_CircleInstance> instances;
			dx11::ArrayBuffer<GPU_CircleInstance> instances_buff;

			// Shaders
			ComPtr<ID3D11PixelShader> circle_ps;
		};
		CircleRender circle_renderer;

		// Created Windows
		std::list<Window> windows;

	public:
		std::string _cache_string;  // used by the gliph position methods

		bool _bruteForceCreateSwapchain(Window& window, ComPtr<IDXGISwapChain1>& swapchain1);

		void _loadCharacterAtlasToTexture();

	public:
		void create();

		// What is the size of each character
		// Where it should be placed
		// How large the container should be to contain the text

		// deprecated
		void findAndPositionGlyphs(
			std::string& text,
			GlyphProperties& props,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void findAndPositionGlyphs(
			utf8string& text,
			GlyphProperties& props,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void findAndPositionGlyphs(
			utf8string& text,
			GlyphProperties& props,
			PositionedCharacters& result);
		
		// deprecated
		void findAndPositionGlyphs(TextProps& props,
			int32_t start_pos_x, int32_t start_pos_y,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void findAndPositionGlyphs(
			int32_t number,
			GlyphProperties& props,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void findAndPositionGlyphs(
			float number, uint32_t decimal_count,
			GlyphProperties& props,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void drawTexts(Window* window, ClipZone& clip_zone, std::vector<TextInstance*>& instances);
		void drawTexts(Window* window, std::vector<TextInstance*>& instances);
		void drawTexts(Window* window, ClipZone& clip_zone, std::vector<TextInstance>& instances);

		void _drawRects(Window* window, RectInstance** instances_data, size_t instances_count);
		void drawRect(Window* window, RectInstance* instance);
		void drawRects(Window* window, std::vector<RectInstance*>& instances);
		void drawRects(Window* window, std::vector<RectInstance>& instances);

		void drawCircles(Window* window, CircleInstance** instances_data, size_t instances_count);
		void drawCircle(Window* window, CircleInstance* instance);

		void drawBorder(Window* window, std::vector<BorderInstance*>& instances);

		void drawArrows(Window* window, std::vector<ArrowInstance*>& instances);

	//	// acquire file handle to detect changes
	//	//void registerStyleFile(io::Path& path);

	//	// void unregisterStyleFile();

		Window* createWindow(Window::CreateInfo& info);
	};


	LRESULT CALLBACK windowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
}
