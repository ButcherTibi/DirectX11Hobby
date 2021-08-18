#pragma once

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// Standard
#include <variant>
#include <atomic>
#include <chrono>
#include <list>
#include <map>
#include <unordered_set>
#include <optional>

// GLM
#include <glm\vec4.hpp>

// Mine
#include "DX11Wrapper.hpp"
#include "GPU_ShaderTypes.hpp"
#include "CharacterAtlas.hpp"
#include "Input.hpp"
#include "CommonProperties.hpp"
#include "RenderingObjects.hpp"
#include "BasicMath.h"


// TODO:
// - tree list
// - button
// - context menu
// - tabs
// - tooltips
// - edit text
// - slider
// - check box
// - dropdown
// - icon
// - image


// NOTES TO SELF:
// - when dealing with graphs, most of the it's better to have a root as it makes vertical traversal symetric


// BAD IDEAS:
// - using instance rendering for each unicode character
//   At first a good idea, vertex and index data computed at font sizing, only write instance data at past layout
//   but mix in element transparency and find that you need to have one drawcall per character
//   times the number of text elements.
//   This sort of rendering only make sense if render whole page and not tens of small labels
// - 


// FORMER BAD IDEAS:
// - Z Index property, its not that usefull and kinda confusing to have separate layout calculation but different display,
//   making the event handling system account for it is hard and kinda slow.
//   What do you do if you have an element that needs to extend past another like a dropdown or a menu ?


// Version 2: Full support for flex type positioning, improved event handling and shortcuts
// Version 3: Simplified the design and generalized the elements, separated elements from the drawing code,
//  removed Z Index feature
// Version 3.5: Readded Z index feature, changed how events are propagated, no longer try to pack gpu_data
//  into large buffers
// Version 4: 
namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Element;

	struct Root;
	class Text;
	struct Stack;
	class Flex;
	class Rect;

	struct MenuItem;
	struct MenuSection;
	class Menu;

	struct StoredElement2;

	// Typedefs
	typedef std::variant<Root, Text, Stack, Flex, Menu, Rect> StoredElement;


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
	class EventsComponent {
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

		// Loops the mouse around the element
		void beginMouseLoopDeltaEffect(Element* elem);

		// hides the mouse and on end restore mouse position to original position
		void beginMouseFixedDeltaEffect(Element* elem);


		// Other

		// get how long is the mouse inside component in milliseconds
		float getInsideDuration();
	};

	struct EventsPassedElement {
		bool allow_inside_event;  // is child allowed 
		Element* elem;
	};


	// Element /////////////////////////////////////////////////////////////////

	namespace ElementType {
		enum {
			ROOT,
			TEXT,
			RELATIVE,
			FLEX,
			MENU,
			RECT
		};
	}

	struct ElementRetainedState {
		std::string id;
		bool used;

		std::array<InternalAnimatedProperty<ElementSize>, 2> size;

		std::array<InternalAnimatedProperty<float>, 2> origin;
		std::array<InternalAnimatedProperty<ElementPosition>, 2> relative_position;
		InternalAnimatedProperty<float> flex_grow;
	};

	// Base element class used by all elements
	class Element {
	public:
		Window* _window;  // pointer to parent window some little things

		StoredElement2* _parent;  // parent element that may determine layout or size

		StoredElement2* _self;

		// children that this element contains
		std::vector<StoredElement2*> _children;

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


		// Relative Wrap

		// the origin of the element expresed as 0% to 100% from computed size
		// works the same as the origin of a mesh in a 3D app
		std::array<float, 2> origin;

		// the position relative to parent expresed in relative or absolute units
		// also works under Window/Root
		std::array<ElementPosition, 2> relative_position;

		// Flex

		// how much to grow an element
		float flex_grow;

	public:
		// Virtuals in order of usage

		// element may implement this to process events
		virtual void _emitEvents(bool& r_allow_inside_events);

		// element may implement this if _size requires computation
		// Example:
		// if element size[0] = Fit then size of the element depends on calculating the size of children
		// if element size cannot be determined ahead of time as in the case of the Menu element
		virtual void _calcSizeAndRelativeChildPositions();

		// implemented to load data to the GPU and draw
		virtual void _draw();
	};

	Element* getElementBase(StoredElement* elem);


	// Container element class for adding children to parent elements

	struct ElementCreateInfo {
		std::string id;

		std::array<AnimatedProperty<ElementSize>, 2> size;
		Z_Index z_index;

		std::array<AnimatedProperty<float>, 2> origin;
		std::array<AnimatedProperty<ElementPosition>, 2> relative_position;
		AnimatedProperty<float> flex_grow;
	};

	struct TextCreateInfo : public ElementCreateInfo {
		std::string text;
		std::string font_family;
		std::string font_style;
		AnimatedProperty<uint32_t> font_size;
		AnimatedProperty<uint32_t> line_height;
		AnimatedProperty<Color> color;

		TextCreateInfo();
	};

	enum class FlexOrientation {
		ROW,
		COLUMN
	};

	enum class FlexSpacing {
		START,
		CENTER,
		END,
		SPACE_BETWEEN,
	};

	struct FlexCreateInfo : public ElementCreateInfo {
		FlexOrientation orientation;
		FlexSpacing items_spacing;
		FlexSpacing lines_spacing;
	};

	struct RectCreateInfo : public ElementCreateInfo {
		AnimatedProperty<Color> color;
	};

	struct MenuCreateInfo : public ElementCreateInfo {
		Color titles_background_color;
		Color titles_highlight_color;

		uint32_t titles_border_thickness;
		Color titles_border_color;
	};

	class Container : public Element {
	public:
		void assign(Element& elem, ElementRetainedState* prev, ElementCreateInfo& next);

		void createText(TextCreateInfo& info);
		Flex* createFlex(FlexCreateInfo& info);
		Rect* createRect(RectCreateInfo& info);
		Menu* createMenu(MenuCreateInfo& info);

		//Rect* createRectangle();
		//RelativeWrap* createRelativeWrap();
		//Flex* createFlex();
		//Menu* createMenu();
	};


	// Root ////////////////////////////////////////////////////////////////////////////////////

	// having a mostly unused root element is helpfull for symentric graph traversal
	struct Root : Container {
		EventsComponent _events;

		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// Text ////////////////////////////////////////////////////////////////////////////////////

	struct TextRetainedState : public ElementRetainedState {		
		InternalAnimatedProperty<uint32_t> font_size;
		InternalAnimatedProperty<uint32_t> line_height;
		InternalAnimatedProperty<Color> color;
	};

	class Text : public Element {
	public:
		std::string text;
		std::string font_family;
		std::string font_style;
		uint32_t font_size;
		uint32_t line_height;
		Color color;

		TextInstance instance;

	public:
		void _calcSizeAndRelativeChildPositions() override;

		void _draw() override;
	};


	// Rectangle //////////////////////////////////////////////////////////////////////////////////

	struct RectRetainedState : public ElementRetainedState {
		InternalAnimatedProperty<Color> color;
	};

	class Rect : public Container {
	public:
		Color color;

	public:
		void _draw() override;
	};


	// Button ///////////////////////////////////////////////////////////////////////////////////




	// DirectX11_Viewport ////////////////////////////////////////////////////////////////////////

	/*class DirectX11_Viewport {
	public:

	};*/


	// Background Element ///////////////////////////////////////////////////////////////////////
	
	struct SurfaceEvent {
		// Resource
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* im_ctx3;

		uint32_t render_target_width;
		uint32_t render_target_height;
		ID3D11RenderTargetView* compose_rtv;

		// Drawcall
		glm::uvec2 viewport_pos;
		glm::uvec2 viewport_size;
	};

	//typedef void(*RenderingSurfaceCallback)(Window* window, StoredElement* source, SurfaceEvent& event, void* user_data);

	//struct BackgroundElement : public Element {
	//	BackgroundColoring coloring;

	//	Color background_color;
	//	AnimatedProperty<glm::vec4> _background_color;

	//	RenderingSurfaceCallback _onRenderingSurface;
	//	void* _surface_event_user_data;

	//	RectRender _rect_render;

	//	EventsComponent _events;

	//public:
	//	void _init();
	//	void _generateGPU_Data() override;
	//	void _draw() override;

	//	void setColorTransition(Color& end_color, uint32_t duration);

	//	void setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data = nullptr);
	//};


	// Stack ///////////////////////////////////////////////////////////////////////

	struct Stack : public Container {
		// Element does not have any properties, but children do

	public:
	//	void _calcSizeAndRelativeChildPositions() override;
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

		EventsComponent _events;

	public:
		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions() override;

		// Events
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);

		void setMouseMoveEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);

		void beginMouseLoopDeltaEffect();
		void beginMouseFixedDeltaEffect();
	};


	// Menu //////////////////////////////////////////////////////////////

	struct SubMenuCreateInfo {
		Color background_color;

		uint32_t border_thickness = 0;
		Color border_color;

		// std::array<uint32_t, 2> shadow_offset;
		// uint32_t shadow_spread;
		// Color shadow_color;

		SubMenuCreateInfo() = default;
		SubMenuCreateInfo(Color& new_background_color);
	};

	struct MenuSectionCreateInfo {
		//Color background_color;

		//Color separator_color;
		//uint32_t separator_thickness;
	};

	struct MenuItemCreateInfo {
		TextProps left_text;
		//TextProps right_text;

		uint32_t top_padding = 0;
		uint32_t bot_padding = 0;
		uint32_t left_padding = 0;
		uint32_t right_padding = 0;
		uint32_t arrow_left_padding = 0;

		Color highlight_color;

		// Arrow
		uint32_t arrow_width = 14;
		uint32_t arrow_height = 14;
		Color arrow_color;
		Color arrow_highlight_color;

		// Events
		EventCallback callback = nullptr;
	};

	
	struct SubMenu {
		std::vector<uint32_t> child_sections;

		SubMenuCreateInfo info;

		Box2D box;

		// Render
		RectInstance _background;
	};

	struct MenuSection {
		std::vector<uint32_t> child_items;

		MenuSectionCreateInfo info;

		Box2D _section_box;

		// Render
		RectInstance _background;
	};

	struct MenuItem {
		uint32_t sub_menu;

		MenuItemCreateInfo info;
	
		Box2D box;

		// Render
		TextInstance _text;
		ArrowInstance _arrow;
	};

	struct MenuVisibleMenus {
		uint32_t menu;
		uint32_t item;
	};

	struct MenuRetainedState : public ElementRetainedState {
		// Graph from last frame
		std::vector<SubMenu> prev_submenus;
		std::vector<MenuSection> prev_sections;
		std::vector<MenuItem> prev_items;

		// Current frame graph
		std::vector<SubMenu> submenus;
		std::vector<MenuSection> sections;
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
		void _emitEvents(bool& allow_inside_events) override;
		void _calcSizeAndRelativeChildPositions() override;
		void _draw() override;


		uint32_t createTitle(MenuItemCreateInfo& info);

		uint32_t createSubMenu(uint32_t parent_item, SubMenuCreateInfo& info);
		uint32_t createSection(uint32_t parent_submenu, MenuSectionCreateInfo& info = nui::MenuSectionCreateInfo());
		uint32_t createItem(uint32_t parent_section, MenuItemCreateInfo& info);
	};


	//



	//////////////////////////////////////////////////////////////////////////////


	struct WindowMessages {
		bool should_close;
		bool is_minimized;
	};

	struct StoredElement2 {
		StoredElement specific_elem;  // where elements are actually stored in memory
		Element* base_elem;  // for easy access

		template<typename T>
		T* get()
		{
			return std::get_if<T>(&specific_elem);
		}
	};

	class Window {
	public:
		Instance* instance;  // parent instance to which this window belongs

		WNDCLASSA window_class;  // Win32 Window class used on creation
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
		
		WindowMessages win_messages;

		// Elements
		std::list<StoredElement2> elements;  // where all the elements in the graph are stored
		Root* root;
		
		std::list<TextRetainedState> text_prevs;
		std::list<FlexRetainedState> flex_prevs;
		std::list<RectRetainedState> rect_prevs;
		std::list<MenuRetainedState> menu_prevs;
	
		// the order in which to draw the elements on screen
		// also used for occlusion in emiting events
		std::map<uint32_t, std::list<Element*>> draw_stacks;

		// Mouse Delta Handling
		enum class DeltaEffectType {
			LOOP,
			HIDDEN
		};
		DeltaEffectType delta_effect;  // type of delta effect to apply, only one delta effect can be present
		
		Element* delta_owner_elem;  // who owns the delta efect

		// initial mouse coordinates at the start of the delta effect (only for HIDDEN)
		uint32_t begin_mouse_x;
		uint32_t begin_mouse_y;

		// Events
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

		struct PassedElement {
			std::array<int32_t, 2> ancestor_pos;
			int32_t ancestor_z_index;
			Element* elem;
		};
		std::vector<PassedElement> _now_pelems;
		std::vector<PassedElement> _next_pelems;

		void _render();

		// void _addElement(Element* parent, Element* new_elem);

	public:
		void createText(TextCreateInfo& info);
		Flex* createFlex(FlexCreateInfo& info);
		Rect* createRectangle(RectCreateInfo& info);
		Menu* createMenu(MenuCreateInfo& info);

		//Stack* createRelativeWrap(Element* parent = nullptr);
		//Menu* createMenu(Element* parent = nullptr);


		// Update
		void update(WindowCallback callback);


		// Events

		// event to be executed before all element events are executed 
		// void setStartEvent(WindowCallback callback, void* user_data = nullptr);

		// event to be executed after all element events are executed
		void setEndEvent(WindowCallback callback, void* user_data = nullptr);

		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void endMouseDeltaEffect();

		// Window

		// get rect of the actual rendered surface excludes border shadows and top bar
		RECT getClientRectangle();

		// Mouse
		bool setLocalMousePosition(uint32_t x, uint32_t y);
		bool untrapMousePosition();
		void setMouseVisibility(bool is_visible);
	};

	 
	extern std::list<Window*> _created_windows;


	struct WindowCreateInfo {
		uint16_t width;
		uint16_t height;
	};

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

		// Gradient Rect Shaders
		//ComPtr<ID3D11PixelShader> rect_flat_fill_ps;
		//ComPtr<ID3D11PixelShader> rect_gradient_linear_ps;

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

		// Created Windows
		std::list<Window> windows;

	public:
		bool _bruteForceCreateSwapchain(Window& window, ComPtr<IDXGISwapChain1>& swapchain1);

		void _loadCharacterAtlasToTexture();

	public:
		void create();

		// What is the size of each character
		// Where it should be placed
		// How large the container should be to contain the text
		void findAndPositionGlyphs(TextProps& props,
			int32_t start_pos_x, int32_t start_pos_y,
			uint32_t& r_width, uint32_t& r_height,
			std::vector<PositionedCharacter>& r_chars);

		void drawTexts(Window* window, std::vector<TextInstance*>& instances);
		
		void drawRects(Window* window, std::vector<RectInstance*>& instances);

		void drawArrows(Window* window, std::vector<ArrowInstance*>& instances);

		void drawBorder(Window* window, std::vector<BorderInstance*>& instances);

		// acquire file handle to detect changes
		//void registerStyleFile(io::Path& path);

		// void unregisterStyleFile();

		Window* createWindow(WindowCreateInfo& info);
	};
}
