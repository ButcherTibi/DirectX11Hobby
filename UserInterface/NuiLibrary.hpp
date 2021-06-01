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
// - tooltips
// - menu arrow
// - menu shortcuts
// - context menu
// - button
// - drop down
// - tabs
// - tree list
// - edit text
// - slider


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
namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Element;
	struct Root;
	struct Text;
	struct RelativeWrap;
	class Grid;
	struct MenuItem;
	class Menu;

	// Typedefs
	typedef std::variant<Root, Text, RelativeWrap, Grid, Menu> StoredElement;


	// Events Component /////////////////////////////////////////////////////////////////
	typedef void (*EventCallback)(Window* window, StoredElement* source, void* user_data);

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

	class EventsComponent {
	public:
		// Internal
		Window* _window;

		MouseEventState _mouse_event_state;

		EventCallback _onMouseEnter;
		void* _mouse_enter_user_data;
		SteadyTime _mouse_enter_time;

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

		void _init(Window* window);

		void _emitInsideEvents(StoredElement* self);
		void _emitOutsideEvents(StoredElement* self);

	public:
		void setMouseEnterEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseHoverEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseMoveEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseLeaveEvent(EventCallback callback, void* user_data = nullptr);

		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data = nullptr);
		void setKeysDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data = nullptr);

		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, void* user_data = nullptr);
		void setKeysHeldDownEvent(EventCallback callback, uint32_t key_0, uint32_t key_1, uint32_t key_2, void* user_data = nullptr);

		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);

		// Loops the mouse around the element
		void beginMouseLoopDeltaEffect(Element* elem);
		void beginMouseFixedDeltaEffect(Element* elem);

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
			RELATIVE_WRAP,
			GRID,
			MENU
		};
	}

	struct ResourceIndexes {
		uint32_t char_vertex_idx;
		uint32_t char_index_idx;
		uint32_t text_inst_idx;

		uint32_t rect_vertex_idx;
		uint32_t rect_index_idx;
	};

	class Element {
	public:
		Window* _window;

		Element* _parent;
		StoredElement* _self;
		std::list<Element*> _children;

		std::array<float, 2> origin;
		std::array<ElementPosition, 2> relative_position;  // also works under Window/Root
		std::array<int32_t, 2> _position;  // calculated position

		uint32_t z_index;  // z_index is not inherited
		uint32_t _z_index;

		std::array<ElementSize, 2> size;
		std::array<uint32_t, 2> _size;  // calculated size

	public:
		void _init();

		// eg. width = 50% of parent or width = 100px
		void _calcSizeRelativeToParent();

		// takes a list of next chilldren and appends it's own children if any
		virtual void _emitEvents(bool& allow_inside_events);

		// calculate size if size requires computation
		// Example:
		// if element size[0] = Fit then size of the element depends on calculating the size of children
		// if element size cannot be determined ahead of time as in the case of the Menu element
		virtual void _calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents);

		virtual void _generateGPU_Data();

		virtual void _draw();
	};


	Element* getElementBase(StoredElement* elem);


	struct Root : Element {
		EventsComponent _events;

		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents) override;
	};


	struct Text : Element {
		std::string text;  // TODO: unicode
		std::string font_family;
		std::string font_style;
		uint32_t font_size;  // TODO: text font size metrics
		uint32_t line_height;
		Color color;
	};


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

	typedef void(*RenderingSurfaceCallback)(Window* window, StoredElement* source, SurfaceEvent& event, void* user_data);

	struct BackgroundElement : public Element {
		BackgroundColoring coloring;

		Color background_color;
		AnimatedProperty<glm::vec4> _background_color;

		RenderingSurfaceCallback _onRenderingSurface;
		void* _surface_event_user_data;

		RectRender _rect_render;

		EventsComponent _events;

	public:
		void _init();
		void _generateGPU_Data() override;
		void _draw() override;

		void setColorTransition(Color& end_color, uint32_t duration);

		void setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data = nullptr);
	};


	// RelativeWrap ///////////////////////////////////////////////////////////////////////

	struct RelativeWrap : public BackgroundElement {
		// Element does not have any properties, but children do

	public:
		void _calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents) override;
	};


	// Grid ///////////////////////////////////////////////////////////////////////////////

	enum class GridOrientation {
		ROW,
		COLUMN
	};

	enum class GridSpacing {
		START,
		CENTER,
		END,
		SPACE_BETWEEN,
	};

	enum class GridSelfAlign {
		START,
		CENTER,
		END,
	};

	class Grid : public BackgroundElement {
	public:
		GridOrientation orientation;
		GridSpacing items_spacing;
		GridSpacing lines_spacing;

	public:
		void _emitEvents(bool& allow_inside_events) override;
		void _calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents) override;

		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);

		void beginMouseLoopDeltaEffect();
		void beginMouseFixedDeltaEffect();
	};


	// Menu //////////////////////////////////////////////////////////////

	struct MenuStyle {
		std::string font_family = "Roboto";
		std::string font_style = "Regular";
		uint32_t font_size = 14;
		uint32_t line_height = 0;
		Color text_color = Color(1.0f, 1.0f, 1.0f);

		uint32_t top_padding = 0;
		uint32_t bot_padding = 0;
		uint32_t left_padding = 0;
		uint32_t right_padding = 0;

		Color menu_background_color;
	};

	// Each Menu Item is both a child and parent
	// it is a child entry of a menu
	// and a parent title of a sub menu
	struct MenuItem {
		MenuItem* _parent;
		std::list<MenuItem*> _children;

		// Label
		std::string text;
		std::string font_family;
		std::string font_style;
		uint32_t font_size;
		uint32_t line_height;
		Color text_color;

		uint32_t top_padding;
		uint32_t bot_padding;
		uint32_t left_padding;
		uint32_t right_padding;
		Box2D _label_box;
		EventCallback label_callback;
		void* label_user_data;

		// Menu
		Color menu_background_color;

		Box2D _menu_box;
	};

	class Menu : public Element {
	public:
		std::list<MenuItem> _items;

		RectRender _menu_background_render;
		RectRender _select_background_render;
		TextRender _label_render;

		Color titles_background_color;
		Color select_background_color;

		// contains all the visible menus
		// first is always root the which is always rendered and does not have any events triggered
		// every menu that is present in this list is rendered
		std::vector<MenuItem*> _visible_menus;

	public:
		void _init();

		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions(std::unordered_set<Element*>& r_next_parents) override;

		void _generateGPU_Data() override;

		void _draw() override;

		MenuItem* addItem(MenuItem* parent, MenuStyle& style);
	};


	struct WindowMessages {
		bool should_close;
		bool is_minimized;
	};


	class Window {
	public:
		Instance* instance;

		WNDCLASSA window_class;
		HWND hwnd;

		// Input data updated by WinProc
		Input input;

		// Window Size
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t surface_width;  // width of the renderable surface
		std::uint32_t surface_height;  // height of the renderable surface
		
		WindowMessages win_messages;

		// Elements
		std::list<StoredElement> elements;

		std::map<uint32_t, std::list<Element*>> draw_stacks;

		// Mouse Delta Handling
		enum class DeltaEffectType {
			LOOP,
			HIDDEN
		};
		DeltaEffectType delta_effect;
		
		Element* delta_owner_elem;
		uint32_t begin_mouse_x;
		uint32_t begin_mouse_y;

		// DirectX 11
		ComPtr<IDXGISwapChain1> swapchain1;
		ComPtr<ID3D11Texture2D> present_img;
		ComPtr<ID3D11RenderTargetView> present_rtv;
	
		GPU_Constants gpu_constants;
		dx11::Buffer cbuff;

		// Viewport
		D3D11_VIEWPORT viewport;

	public:
		void _updateCPU();

		void _render();

	public:
		Text* createText(Element* parent = nullptr);
		RelativeWrap* createRelativeWrap(Element* parent = nullptr);
		Grid* createGrid(Element* parent = nullptr);
		Menu* createMenu(Element* parent = nullptr);

		// Events
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void endMouseDeltaEffect();

		// Window
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
		
		// Timing
		SteadyTime frame_start_time;
		//SteadyTime prev_frame_start_time;
		float delta_time;  // the total duration between this frame and previous
		uint32_t min_frame_duration_ms;

		// DirectX 11 Context
		ComPtr<IDXGIFactory2> factory2;
		ComPtr<IDXGIFactory7> factory7;
		ComPtr<IDXGIAdapter> adapter;
		ComPtr<ID3D11Device> dev1;
		ComPtr<ID3D11Device5> dev5;
		ComPtr<ID3D11DeviceContext> im_ctx1;
		ComPtr<ID3D11DeviceContext3> im_ctx3;

		// Simple Vertex Shader
		std::vector<char> simple_vs_cso;
		ComPtr<ID3D11VertexShader> simple_vs;
		ComPtr<ID3D11InputLayout> simple_input_layout;

		// Character Shaders
		std::vector<char> char_vs_cso;
		ComPtr<ID3D11VertexShader> char_vs;
		ComPtr<ID3D11InputLayout> char_input_layout;

		ComPtr<ID3D11PixelShader> char_ps;

		// Gradient Rect Shaders
		//ComPtr<ID3D11PixelShader> rect_flat_fill_ps;
		//ComPtr<ID3D11PixelShader> rect_gradient_linear_ps;

		// Rect Shaders
		ComPtr<ID3D11PixelShader> rect_ps;

		// Character Atlas
		CharacterAtlas char_atlas;
		dx11::Texture char_atlas_tex;
		ComPtr<ID3D11SamplerState> char_atlas_sampler;

		// Rasterizer State
		ComPtr<ID3D11RasterizerState> solid_back_rs;

		// Blend State
		ComPtr<ID3D11BlendState> blend_state;

		// Created Windows
		std::list<Window> windows;

	public:
		bool _bruteForceCreateSwapchain(Window& window, ComPtr<IDXGISwapChain1>& swapchain1);

	public:
		void create();

		void loadCharacterAtlasToTexture();

		Window* createWindow(WindowCreateInfo& info);

		void update();
	};
}
