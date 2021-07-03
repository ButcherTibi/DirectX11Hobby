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
	class Flex;
	struct MenuItem;
	class Menu;
	struct UpdateChange;

	// Typedefs
	typedef std::variant<Root, Text, RelativeWrap, Flex, Menu> StoredElement;


	// Events Component /////////////////////////////////////////////////////////////////
	typedef void (*EventCallback)(Window* window, StoredElement* source, void* user_data);
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
		void _emitInsideEvents(StoredElement* self);

		// emit events where the mouse is outside the element
		// for now only leave event fits this case
		void _emitOutsideEvents(StoredElement* self);

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
			MENU
		};
	}

	namespace ChangedElementType {
		enum {
			FLEX,
			MENU
		};
	}

	struct ChangedElement {
		std::optional<uint32_t> z_index;
		std::optional<std::array<ElementSize, 2>> size;
	};


	// Base element class used by all elements
	class Element {
	public:
		Window* _window;  // pointer to parent window some little things

		Element* _parent;  // parent element that may determine layout or size

		// reference to self from the children if the parent used only for deletion
		std::list<Element*>::iterator _self_children;

		// reference to self as a specific element from window elements
		std::list<StoredElement>::iterator _self_elements;

		// children that this element contains
		std::list<Element*> _children;

		// the origin of the element expresed as 0% to 100% from computed size
		// used to center the element in relative layout, work the same as the origin of a mesh in a 3D app
		std::array<float, 2> origin;

		// the position relative to parent expresed in relative or absolute units
		// also works under Window/Root
		std::array<ElementPosition, 2> relative_position;

		// the computed position to be sent to the GPU
		std::array<int32_t, 2> _position;

		// 
		float flex_grow;

		// the order in which to render the element and it's descendants
		// is 0 by default meaning inherit from parent
		uint32_t z_index;

		// the computed Z index that controls in which draw stack will this element be assigned
		uint32_t _z_index;

		// absolute size in pixels
		// relative size as 0% to 100% of parent computed size
		// fit size is based on the size of children
		std::array<ElementSize, 2> size;

		// the computed size used in layout calculation
		std::array<uint32_t, 2> _size;

		UpdateChange* _update;

	public:
		void _init();

		void _ensureHasChange();

		ChangedElement& _ensureChangedElement();


		// Update

		void setZ_Index(uint32_t new_z_index);
		
		void setSize(ElementSize x, ElementSize y);

		void getSize(ElementSize& r_x, ElementSize& r_y);


		// Virtuals in order of usage

		// eg. width = 50% of parent or width = 100px
		// only called in one location
		void _calcSizeRelativeToParent();

		// element may implement this in order process events
		virtual void _emitEvents(bool& r_allow_inside_events);

		// element may implement this if size requires computation
		// Example:
		// if element size[0] = Fit then size of the element depends on calculating the size of children
		// if element size cannot be determined ahead of time as in the case of the Menu element
		virtual void _calcSizeAndRelativeChildPositions();

		// element may use this to generate data to be loaded on the GPU
		virtual void _generateGPU_Data();

		// implemented to load data to the GPU and draw
		virtual void _draw();
	};

	Element* getElementBase(StoredElement* elem);


	// having a mostly unused root element is helpfull for symentric graph traversal
	struct Root : Element {
		EventsComponent _events;

		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions() override;
	};


	// TODO: reimplement
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
		void _calcSizeAndRelativeChildPositions() override;
	};


	// Flex ///////////////////////////////////////////////////////////////////////////////

	class Flex : public BackgroundElement {
	public:
		enum class Orientation {
			ROW,
			COLUMN
		};

		enum class Spacing {
			START,
			CENTER,
			END,
			SPACE_BETWEEN,
		};

		// TODO: not implmented
		enum class SelfAlign {
			START,
			CENTER,
			END,
		};

		Orientation orientation;
		Spacing items_spacing;
		Spacing lines_spacing;

	public:
		struct Change {
			std::optional<Flex::Orientation> orientation;
		};
		Change& ensureChangedFlex();

		// Updates
		Orientation getOrientation();
		void setOrientation(Orientation new_orientation);

		// Events
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyHeldDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void setKeyUpEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);

		void setMouseMoveEvent(EventCallback callback, void* user_data = nullptr);
		void setMouseScrollEvent(EventCallback callback, void* user_data = nullptr);

		void beginMouseLoopDeltaEffect();
		void beginMouseFixedDeltaEffect();

		void _emitEvents(bool& allow_inside_events) override;
		void _calcSizeAndRelativeChildPositions() override;
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
		Menu* menu;

		MenuItem* parent;
		std::list<MenuItem*> children;

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

	public:
		void setItemText(std::string text);

		void setItemCallback(EventCallback callback);

		MenuItem* addItem(MenuStyle& style);
	};

	namespace MenuItemChangeType {
		enum {
			ADD,
			UPDATE,
			REMOVE
		};
	}

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

		struct Change {

			struct AddItem {
				MenuItem* parent;
				MenuItem* item;  // where was the element added
			};

			struct UpdateItem {
				MenuItem* item;

				std::optional<std::string> text;
				std::optional<EventCallback> callback;
			};
			std::vector<std::variant<AddItem, UpdateItem>> item_changes;

			std::optional<Color> titles_background_color;
			std::optional<Color> select_background_color;
		};

		Change& _ensureChangedMenu();

		Change::UpdateItem& _addItemUpdatedChange(MenuItem* item);

		MenuItem* _addItem(MenuItem* parent, MenuStyle& style);


		// Updates

		void setTitleBackColor(Color new_color);

		void setSelectBackColor(Color new_color);

		MenuItem* addTitle(MenuStyle& style);


		// Virtuals
		void _emitEvents(bool& allow_inside_events) override;

		void _calcSizeAndRelativeChildPositions() override;

		void _generateGPU_Data() override;

		void _draw() override;
	};

	//////////////////////////////////////////////////////////////////////////////


	struct WindowMessages {
		bool should_close;
		bool is_minimized;
	};

	namespace ElementGraphChangeType {
		enum {
			ADD,
			UPDATE,  // update element fields
			REMOVE  // remove element
		};
	}

	// added elements just get added to the elements list directly and only need to be linked
	struct AddChange {
		Element* parent;
		Element* elem;
	};

	struct UpdateChange {
		StoredElement* dest;  // who to update

		// what to update
		ChangedElement source_elem;
		std::variant<Flex::Change, Menu::Change> source;
	};

	// where to delete
	struct DeleteChange {
		std::list<StoredElement>::iterator target;
	};


	class Window {
	public:
		Instance* instance;  // parent instance to which this window belongs

		WNDCLASSA window_class;  // Win32 Window class used on creation
		HWND hwnd;  // handle to the window

		// Input data updated by WinProc
		Input input;

		// Window Size
		std::uint32_t width;
		std::uint32_t height;
		std::uint32_t surface_width;  // width of the renderable surface
		std::uint32_t surface_height;  // height of the renderable surface
		
		WindowMessages win_messages;

		// Elements
		std::list<StoredElement> elements;  // where all the elements in the graph are stored
		
		std::vector<std::variant<AddChange, UpdateChange, DeleteChange>> changes;  // scheduled changes to be aplied to elements

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
		Flex* createGrid(Element* parent = nullptr);
		Menu* createMenu(Element* parent = nullptr);

		// deletes an element from graph and leaves it detached
		void deleteElement(Element* elem);

		// delete all elements (does not delete root)
		void deleteAllElements();

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
