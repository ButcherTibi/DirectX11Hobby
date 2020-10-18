#pragma once

// Standard
#include <variant>
#include <atomic>
#include <chrono>

// GLM
#include <glm\vec4.hpp>

// Mine
#include "DX11Wrapper.hpp"
#include "GPU_ShaderTypes.hpp"
#include "FontRasterization.hpp"
#include "WindowInput.hpp"


/* TODO:
- on key up
- handle Alt, F10
- fullscreen support
- multiple fonts
- flex
- wrap self origin, child pos can refer to center of child
- z order
- multiple windows, needs atomics
- the DirectX 11 character atlas texture does NOT resize with atlas
*/


namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Wrap;
	class Flex;
	class Text;
	class Surface;
	class Node;


	enum class ElementSizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE,
		FIT
	};

	struct ElementSize {
		ElementSizeType type;
		float size;

	public:
		ElementSize& operator=(uint32_t size_px);
		ElementSize& operator=(float percentage);
		void setAbsolute(float size);
	};


	#undef OVERFLOW
	enum class Overflow {
		OVERFLOW,
		CLIP
	};


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

		void setRGBA_UNORM(float r, float g, float b, float a = 1.0f);
	};


	enum class FlexDirection {
		ROW,
		COLUMN,
	};

	enum class FlexWrap {
		NO_WRAP,
		WRAP,
	};

	enum class FlexAxisAlign {
		START,
		END,
		CENTER,
		SPACE_BETWEEN,
	};

	enum class FlexCrossAxisAlign {
		START,
		END,
		CENTER,
		PARENT,
	};

	enum class FlexLinesAlign {
		START,
		END,
		CENTER,
		SPACE_BETWEEN,
	};


	class NodeComponent {
	public:
		Window* window;

		Node* this_elem;

	public:
		Text* addText();
		Wrap* addWrap();
		Flex* addFlex();
		Surface* addSurface();
	};


	struct MouseHoverEvent {
		Node* source;
		float duration;
		void* user_ptr;
	};

	struct MouseEnterEvent {
		Node* source;
		void* user_ptr;
	};

	struct MouseLeaveEvent {
		Node* source;
		void* user_ptr;
	};

	struct MouseMoveEvent {
		Node* source;
		void* user_ptr;
	};

	struct KeyDownEvent {
		Node* source;
		void* user_ptr;
	};

	typedef void (*MouseHoverCallback)(MouseHoverEvent& hover_event);
	typedef void (*MouseEnterCallback)(MouseEnterEvent& hover_event);
	typedef void (*MouseLeaveCallback)(MouseLeaveEvent& hover_event);
	typedef void (*MouseMoveCallback)(MouseMoveEvent& move_event);
	typedef void (*KeyDownCallback)(KeyDownEvent& key_down_event);

	struct KeyDown {
		uint32_t key;
		KeyDownEvent event;
		KeyDownCallback callback;
	};

	class CommonEventsComponent {
	public:
		Window* window;
		Node* source;
		
		uint32_t last_mouse_x;
		uint32_t last_mouse_y;
		SteadyTime mouse_enter_time;
		bool mouse_entered;
		bool mouse_left;

		// Event State
		MouseHoverEvent hover_event;
		MouseEnterEvent enter_event;
		MouseLeaveEvent leave_event;
		MouseMoveEvent move_event;

		// Callbacks
		void (*onMouseHover)(MouseHoverEvent& hover_event);
		void (*onMouseEnter)(MouseEnterEvent& enter_event);
		void (*onMouseLeave)(MouseLeaveEvent& leave_event);
		void (*onMouseMove)(MouseMoveEvent& move_event);
		std::list<KeyDown> keys_down;

	public:

	public:
		void create(Window* wnd, Node* node);

		void emitInsideEvents();
		void emitOutsideEvents();

		void setMouseHoverEvent(MouseHoverCallback callback, void* user_ptr);
		void setMouseEnterEvent(MouseEnterCallback callback, void* user_ptr);
		void setMouseLeaveEvent(MouseLeaveCallback callback, void* user_ptr);
		void setMouseMoveEvent(MouseMoveCallback callback, void* user_ptr);

		bool addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr);
		bool removeKeyDownEvent(uint32_t key);
	};


	class Root {
	public:
		NodeComponent node_comp;
	};


	struct WrapDrawcall {
		GPU_WrapInstance instance;

		uint32_t instance_idx;
	};

	class Wrap {
	public:
		// General
		NodeComponent _node_comp;
		
		// Particular
		WrapDrawcall _drawcall;

	public:
		glm::uvec2 pos;
		ElementSize width;
		ElementSize height;
		Overflow overflow;  // CURSED: assigning a default value causes undefined behaviour, memory coruption
		Color background_color;

	public:
		Text* addText();
		Wrap* addWrap();
		Surface* addSurface();

		void setOnMouseEnterEvent(MouseEnterCallback callback, void* user_data = nullptr);
		void setOnMouseHoverEvent(MouseHoverCallback callback, void* user_data = nullptr);
		void setOnMouseLeaveEvent(MouseLeaveCallback callback, void* user_data = nullptr);
		void setOnMouseMoveEvent(MouseMoveCallback callback, void* user_data = nullptr);
		bool addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr = nullptr);
		bool removeKeyDownEvent(uint32_t key);
	};


	class Flex {
	public:
		// Internal
		NodeComponent node_comp;

	public:
		FlexDirection direction;
		FlexWrap wrap;
		FlexAxisAlign axis_align;
		FlexLinesAlign lines_align;

	public:

	};


	struct CharacterDrawcall {
		Character* chara;

		std::vector<GPU_CharacterInstance> instances;

		uint32_t instance_idx;
	};

	class Text {
	public:  // Internal
		// Generic
		NodeComponent _node_comp;

		// Particular
		std::vector<CharacterDrawcall> _drawcalls;

	public:
		std::u32string text;
		glm::uvec2 pos;
		uint32_t size;
		uint32_t line_height;
		Color color;

	public:
		void setOnMouseEnterEvent(MouseEnterCallback callback, void* user_data = nullptr);
		void setOnMouseHoverEvent(MouseHoverCallback callback, void* user_data = nullptr);
		void setOnMouseLeaveEvent(MouseLeaveCallback callback, void* user_data = nullptr);
		void setOnMouseMoveEvent(MouseMoveCallback callback, void* user_data = nullptr);
		bool addKeyDownEvent(KeyDownCallback callback, uint32_t key, void* user_ptr = nullptr);
		bool removeKeyDownEvent(uint32_t key);
	};




	struct SurfaceEvent {
		Surface* surface;
		void* user_data;

		uint32_t surface_width;
		uint32_t surface_height;

		// Resource
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* de_ctx3;

		// ID3D11ShaderResourceView* compose_srv;
		ID3D11ShaderResourceView* parent_clip_mask_srv;
		ID3D11ShaderResourceView* next_parents_clip_mask_srv;

		ID3D11RenderTargetView* compose_rtv;
		ID3D11RenderTargetView* parent_clip_mask_rtv;
		ID3D11RenderTargetView* next_parents_clip_mask_rtv;

		// Drawcall
		glm::vec2 pos;
		glm::vec2 size;
		uint32_t parent_clip_id;
		uint32_t child_clip_id;
	};

	typedef ErrStack(*RenderingSurfaceCallback)(SurfaceEvent& event);

	class Surface {
	public:
		// Internal
		NodeComponent _node_comp;

		SurfaceEvent _event;

	public:
		glm::uvec2 pos;
		ElementSize width;
		ElementSize height;
		Overflow overflow;

		RenderingSurfaceCallback callback;
		void* user_data;
	};

	namespace ElementType {
		enum {
			ROOT,
			WRAP,
			FLEX,
			TEXT,
			SURFACE,
		};
	}

	class Node {
	public:
		std::variant<Root, Wrap, Flex, Text, Surface> elem;

		// Common Components
		BoundingBox2D<uint32_t> collider;
		CommonEventsComponent event_comp;
		uint32_t layer_idx;

		// Relations
		Node* parent;
		std::list<Node*> children;

	public:
		Root* createRoot();
		Wrap* createWrap();
		Flex* createFlex();
		Text* createText();
		Surface* createSurface();
	};


	struct WindowCrateInfo {
		uint16_t width;
		uint16_t height;
	};

	struct AncestorProps {
		uint32_t width;
		uint32_t height;
		uint32_t clip_width;
		uint32_t clip_height;
		uint32_t clip_mask;
	};

	struct DescendantProps {
		glm::uvec2* pos;
		uint32_t width;
		uint32_t height;
	};

	class Window {
	public:
		WNDCLASS window_class;
		HWND hwnd;

		// Window Procedure Messages
		Input input;
		uint32_t mouse_x;
		uint32_t mouse_y;

		std::uint32_t width;
		std::uint32_t height;
		RECT client_rect;
		std::uint32_t surface_width;
		std::uint32_t surface_height;
	
		bool minimized;
		bool close;

		// UI
		uint32_t clip_mask_id;
		std::list<Node> nodes;

		// Rendering Data
		bool rendering_configured;

		uint32_t char_instance_count;
		uint32_t wrap_instance_count;
		
		std::vector<GPU_CharacterVertex> char_verts;
		std::vector<uint32_t> chars_idxs;
		std::vector<GPU_CharacterInstance> char_instances;
		GPU_CommonsUniform common_uniform;
		std::array<GPU_WrapVertex, 4> wrap_verts;
		std::array<uint16_t, 6> wrap_idxs;
		std::vector<GPU_WrapInstance> wrap_instances;

		// DirectX 11
		ComPtr<IDXGIFactory2> factory;
		ComPtr<IDXGIAdapter> adapter;
		ComPtr<ID3D11Device> _dev;
		ComPtr<ID3D11Device5> dev5;
		ComPtr<ID3D11DeviceContext3> de_ctx3;
		ComPtr<ID3D11DeviceContext> _im_ctx;
		ComPtr<ID3D11DeviceContext4> im_ctx4;
		ComPtr<IDXGISwapChain> swapchain;

		ComPtr<ID3D11Texture2D> present_img;
		ComPtr<ID3D11Texture2D> parents_clip_mask_img;
		ComPtr<ID3D11Texture2D> next_parents_clip_mask_img;
		ComPtr<ID3D11Texture2D> chars_atlas_tex;

		ComPtr<ID3D11SamplerState> chars_atlas_sampler;

		ComPtr<ID3D11ShaderResourceView> next_parents_clip_mask_srv;
		ComPtr<ID3D11ShaderResourceView> parents_clip_mask_srv;
		ComPtr<ID3D11ShaderResourceView> chars_atlas_srv;

		ComPtr<ID3D11RenderTargetView> present_rtv;
		ComPtr<ID3D11RenderTargetView> parents_clip_mask_rtv;
		ComPtr<ID3D11RenderTargetView> next_parents_clip_mask_rtv;
	
		dx11::Buffer wrap_vbuff;
		dx11::Buffer wrap_idxbuff;
		dx11::Buffer wrap_instabuff;
		dx11::Buffer chars_vbuff;
		dx11::Buffer chars_idxbuff;
		dx11::Buffer chars_instabuff;
		dx11::Buffer cbuff;

		ComPtr<ID3D11InputLayout> wrap_input_layout;
		ComPtr<ID3D11InputLayout> chars_input_layout;

		ComPtr<ID3D11VertexShader> wrap_vs;
		ComPtr<ID3D11VertexShader> chars_vs;
		ComPtr<ID3D11VertexShader> all_vs;

		ComPtr<ID3D11RasterizerState> rasterizer_state;

		ComPtr<ID3D11PixelShader> wrap_ps;
		ComPtr<ID3D11PixelShader> chars_ps;
		ComPtr<ID3D11PixelShader> copy_parents_ps;

		ComPtr<ID3D11BlendState> blend_state;

	public:
		Instance* instance;

	public:
		void _emitEvents();

		void _calculateLayoutAndDrawcalls(Node* node, AncestorProps& ancestor,
			DescendantProps& r_descendant);
		void _calcGlobalPositions(Node* node, glm::uvec2 pos);

		ErrStack _updateCPU_Data();
		ErrStack _loadCPU_DataToGPU();
		ErrStack _resizeAllAtachments();
		ErrStack _render();

	public:
		Wrap* addWrap();
		Text* addText();
		Surface* addSurface();
	};

	 
	extern std::list<Window> windows;


	class Instance {
	public:	
		HINSTANCE _hinstance;

		HCURSOR _arrow_cursor;

		std::vector<char> _wrap_vs_cso;
		std::vector<char> _chars_vs_cso;
		std::vector<char> _all_vs_cso;

		std::vector<char> _wrap_ps_cso;
		std::vector<char> _chars_ps_cso;
		std::vector<char> _copy_parents_ps_cso;

		CharacterAtlas _char_atlas;

	public:
		ErrStack create();

		ErrStack createWindow(WindowCrateInfo& info, Window*& r_window);

		ErrStack update();

		~Instance();
	};
}
