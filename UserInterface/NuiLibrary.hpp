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
#include "CharacterAtlas.hpp"
#include "Input.hpp"

// Window Components
#include "EventComp.hpp"
#include "NodeComp.hpp"


/* TODO:
- create high-level UI elements, complete menu with submenus and modifiable orientation
- handle Alt, F10
- fullscreen support
- 60fps limit
- multiple fonts
- multiple windows, needs atomics
- the DirectX 11 character atlas texture does NOT resize with atlas
*/


namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Wrap;
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
		ElementSize& operator=(int32_t size_px);
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


	class Root : public NodeComp, public EventComp {
		// methods are on window
	};


	struct WrapDrawcall {
		GPU_WrapInstance instance;
		uint32_t instance_idx;
	};

	class Wrap : public NodeComp, public EventComp {
	public:
		WrapDrawcall _drawcall;

	public:
		glm::uvec2 pos;
		ElementSize width;
		ElementSize height;
		Overflow overflow;  // CURSED: assigning a default value causes undefined behaviour, memory coruption
		Color background_color;

	public:
	};


	struct CharacterDrawcall {
		Character* chara;
		std::vector<GPU_CharacterInstance> instances;
		uint32_t instance_idx;
	};

	class Text : public EventComp {
	public:
		std::vector<CharacterDrawcall> _drawcalls;

	public:
		std::u32string text;
		glm::uvec2 pos;
		uint32_t size;
		uint32_t line_height;
		Color color;

	public:
	};


	struct SurfaceEvent {
		Surface* surface;
		void* user_data;

		// Resource
		ID3D11Device5* dev5;
		ID3D11DeviceContext3* im_ctx3;
		ID3D11DeviceContext3* de_ctx3;

		uint32_t render_target_width;
		uint32_t render_target_height;
		ID3D11RenderTargetView* compose_rtv;

		// Drawcall
		glm::uvec2 viewport_pos;
		glm::uvec2 viewport_size;
	};

	typedef ErrStack(*RenderingSurfaceCallback)(SurfaceEvent& event);

	class Surface : public NodeComp, public EventComp {
	public:
		SurfaceEvent _event;

	public:
		glm::uvec2 pos;  // where to render surface on screen
		ElementSize width;  // not used
		ElementSize height;  // not used

		RenderingSurfaceCallback gpu_callback;
		void* user_data;

	public:
	};

	namespace ElementType {
		enum {
			ROOT,
			WRAP,
			TEXT,
			SURFACE,
		};
	}

	class Node {
	public:
		std::variant<Root, Wrap, Text, Surface> elem;

		// Common Components
		BoundingBox2D<uint32_t> collider;
		uint32_t layer_idx;

		// Relations
		Node* parent;
		std::list<Node*> children;

	public:
		Root* createRoot();
		Wrap* createWrap();
		Text* createText();
		Surface* createSurface();

		EventComp* getCommonEventComponent();
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
		SteadyTime start_time;
		float delta_time;

		Input input;

		std::uint32_t width;
		std::uint32_t height;

		std::uint32_t surface_width;  // width of the renderable surface
		std::uint32_t surface_height;  // height of the renderable surface
	
		bool minimized;
		bool close;

		// UI
		uint32_t clip_mask_id;
		std::list<Node> nodes;

		Node* mouse_delta_owner;  // set by the event component

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
		ComPtr<ID3D11DeviceContext3> im_ctx3;
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

		RECT getClientRectangle();

		bool setLocalMousePosition(uint32_t x, uint32_t y);

		bool trapLocalMousePosition(BoundingBox2D<uint32_t>& box);
		bool untrapMousePosition();
		
		void hideMousePointer(bool hide = true);
		void getMouseDelta(int32_t& mouse_delta_x, int32_t& mouse_delta_y);
	};

	 
	extern std::list<Window> windows;


	struct WindowCreateInfo {
		uint16_t width;
		uint16_t height;
	};

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

		ErrStack createWindow(WindowCreateInfo& info, Window*& r_window);

		ErrStack update();

		~Instance();
	};
}
