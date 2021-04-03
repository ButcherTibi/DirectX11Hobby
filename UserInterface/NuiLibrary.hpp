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

// GLM
#include <glm\vec4.hpp>

// Mine
#include "DX11Wrapper.hpp"
#include "GPU_ShaderTypes.hpp"
#include "CharacterAtlas.hpp"
#include "Input.hpp"


/* TODO:
- 60fps limit
- fullscreen support
- handle Alt, F10
*/


namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	struct Root;
	struct Text;
	struct RelativeWrap;
	struct Grid;

	// Typedefs
	typedef std::variant<Root, Text, RelativeWrap, Grid> StoredElement;

	namespace ElementType {
		enum {
			ROOT,
			TEXT,
			RELATIVE_WRAP,
			GRID
		};
	}


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
		ElementSize& operator=(int32_t size_px);
		ElementSize& operator=(float percentage);
	};


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


	enum class ElementZ_IndexType {
		INHERIT,
		SET
	};

	struct ElementZ_Index {
		ElementZ_IndexType type;
		int32_t z_index;

	public:
		ElementZ_Index& operator=(int32_t z_index);
	};


	//#undef OVERFLOW
	//enum class Overflow {
	//	OVERFLOW,
	//	CLIP
	//};


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


	struct ColorStep {
		Color color;
		float pos;

		ColorStep() = default;
		ColorStep(Color& new_color);
	};


	enum class BackgroundColoring {
		NONE,
		FLAT_FILL,
		LINEAR_GRADIENT,
		// STRIPE
		// SMOOTHSTEP MULTI
		// QUADRATIC/CUBIC BEZIER MULTI
		RENDERING_SURFACE
	};


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

	// Events /////////////////////////////////////////////////////////////////

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

	// Transitions ////////////////////////////////////////////////////////////

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

	struct ColorStepAnimated {
		AnimatedProperty<glm::vec4> color;
		AnimatedProperty<float> pos;
	};

	struct BackgroundAnimated {
		AnimatedProperty<float> gradient_angle;
		std::array<ColorStepAnimated, 8> colors;
	};

	
	// Common properties for all elements
	class Element {
	public:
		std::array<float, 2> origin;
		std::array<ElementPosition, 2> relative_position;  // also works under Window

		// Size
		/*glm::vec4 margins;
		glm::vec4 borders;
		Color border_color;
		glm::vec4 padding;*/
		std::array<ElementSize, 2> size;
		
		// Order
		ElementZ_Index z_index;

		// Events
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

		// Other
		void beginMouseDelta();

		float getInsideDuration();

	public:
		// Internal
		Window* _window;

		StoredElement* _parent;
		StoredElement* _self;
		std::list<StoredElement*> _children;

		std::array<uint32_t, 2> _size;
		std::array<int32_t, 2> _position;

		// Events
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

		// some elements, because they are inside a parent who is outside of the mouse are skipped initially when emiting events
		bool _events_emited;

	public:
		void _init();

		void _emitInsideEvents();
		void _emitOutsideEvents();

		void _calcFirstPassSize(uint32_t axis);
	};

	Element* getElementBase(StoredElement* elem);


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

	struct BackgroundElement : Element {
		BackgroundColoring coloring;
		std::vector<ColorStep> colors;
		float gradient_angle;

		void setColorTransition(Color& end_color, uint32_t idx, uint32_t duration);
		void setColorPositionTransition(float end_position, uint32_t idx, uint32_t duration);
		void setGradientAngleTransition(float end_gradient, uint32_t duration);

		void setRenderingSurfaceEvent(RenderingSurfaceCallback callback, void* user_data = nullptr);

	public:
		BackgroundAnimated _anim;

		// Drawcall
		dx11::IndexedDrawcallParams _drawcall;

		// Instance
		std::array<DirectX::XMFLOAT4, 8> _colors;
		std::array<float, 8> _color_lenghts;

		DirectX::XMFLOAT2 _center;
		float _gradient_angle;

		// Events
		SurfaceEvent _surface_event;
		RenderingSurfaceCallback _onRenderingSurface;
		void* _surface_event_user_data;

	public:
		void _init();
		void _generateGPU_Data(uint32_t& rect_verts_idx, uint32_t& rect_idxs_idx);
		void _draw();
	};


	struct Root : Element {

	};


	struct Text : Element {
		std::string text;  // TODO: unicode
		std::string font_family;
		std::string font_style;
		uint32_t font_size;  // TODO: text font size metrics
		uint32_t line_height;
		Color color;

	public:
		// Internal

		// because characters must be rendered with transparency to look good, each text must be a separate drawcall(s) and be depth sorted
		// doing instance rendering for each letter in label is bad, so just write the vertex and index buffer by hand

		// Drawcall
		uint32_t _vertex_start_idx;
		uint32_t _vertex_count;
		uint32_t _index_start_idx;
		uint32_t _index_count;
		uint32_t _instance_start_idx;
	};


	struct RelativeWrap : BackgroundElement {
		// Element does not have any properties, but children do
	};


	struct Grid : BackgroundElement {
		GridOrientation orientation;
		GridSpacing items_spacing;
		GridSpacing lines_spacing;
	};


	struct WindowMessages {
		bool should_close;
		bool is_minimized;
	};

	struct GridLine {
		uint32_t end_idx;
		uint32_t count;

		uint32_t line_length;
		uint32_t line_thickness;
	};

	struct LayoutMemoryCache {
		std::vector<StoredElement*> stored_elems_0;
		std::vector<StoredElement*> stored_elems_1;
		std::vector<StoredElement*> stored_elems_2;

		std::vector<GridLine> lines;
	};

	class Window {
	public:
		Instance* instance;

		WNDCLASS window_class;
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

		LayoutMemoryCache layout_mem_cache;

		// Z Index sorted list of elements
		std::map<int32_t, std::list<StoredElement*>> render_stacks;

		int32_t delta_trap_top;
		int32_t delta_trap_bot;
		int32_t delta_trap_left;
		int32_t delta_trap_right;
		Element* delta_owner_elem;

		// DirectX 11
		ComPtr<IDXGISwapChain1> swapchain1;
		ComPtr<ID3D11Texture2D> present_img;
		ComPtr<ID3D11RenderTargetView> present_rtv;

		// Atachments

		// Textures
		ComPtr<ID3D11Texture2D> chars_atlas_tex;
		ComPtr<ID3D11ShaderResourceView> chars_atlas_srv;
		ComPtr<ID3D11SamplerState> chars_atlas_sampler;
	
		// Buffers
		std::vector<GPU_CharacterVertex> char_verts;
		std::vector<uint32_t> char_idxs;
		std::vector<GPU_TextInstance> text_instances;

		dx11::Buffer char_vbuff;
		dx11::Buffer char_idxbuff;
		dx11::Buffer text_instabuff;
		
		std::vector<GPU_RectVertex> rect_verts;
		std::vector<uint32_t> rect_idxs;

		dx11::Buffer rect_vbuff;
		dx11::Buffer rect_idxbuff;
		dx11::ConstantBuffer rect_dbuff;

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

		// Event
		void setKeyDownEvent(EventCallback callback, uint32_t key, void* user_data = nullptr);
		void endMouseDelta();

		// Window
		RECT getClientRectangle();

		// Mouse
		bool setLocalMousePosition(uint32_t x, uint32_t y);
		bool trapLocalMousePosition(BoundingBox2D<uint32_t>& box);
		bool untrapMousePosition();
		void hideMousePointer(bool hide = true);
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

		// Character Shaders
		std::vector<char> char_vs_cso;
		ComPtr<ID3D11VertexShader> char_vs;
		ComPtr<ID3D11InputLayout> char_input_layout;

		ComPtr<ID3D11PixelShader> char_ps;

		// Rect Shaders
		std::vector<char> rect_vs_cso;
		ComPtr<ID3D11VertexShader> rect_vs;
		ComPtr<ID3D11InputLayout> rect_input_layout;

		ComPtr<ID3D11PixelShader> rect_flat_fill_ps;
		ComPtr<ID3D11PixelShader> rect_gradient_linear_ps;

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
