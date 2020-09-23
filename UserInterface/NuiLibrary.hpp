#pragma once

// GLM
#include <glm\vec4.hpp>

#include "DX11Wrapper.hpp"
#include "GPU_ShaderTypes.hpp"
#include "FontRasterization.hpp"


namespace nui {

	// Forward declarations
	class Instance;
	class Window;
	class Wrap;
	class Flex;
	class Text;
	class Node;


	enum class ElementSizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE,
		FIT
	};

	struct ElementSize {
		ElementSizeType type = ElementSizeType::FIT;
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

	/* TODO:
	- atomatic rasterized character set
	- TextureAtlas resizes if to small to 2X until <= 64K
	- fix overflow by parents += nextparents at end of layer procesing

	- events void event(EventContext event_ctx, void* user_data)

	- multiple fonts, font name = filename no support for style family
	- flex
	- wrap self origin, child pos can refer to center of child
	- z order
	*/

	class NodeComponent {
	public:
		Window* window;

		Node* this_elem;

	public:
		Text* addText();
		Wrap* addWrap();
		Flex* addFlex();
	};


	class RectColider {
	public:
		BoundingBox2D<uint32_t> collider;
	};


	struct WrapDrawcall {
		GPU_WrapInstance instance;

		uint32_t instance_idx;
	};

	class Wrap {
	public:
		// Internal
		NodeComponent node_comp;
		WrapDrawcall drawcall;

	public:
		glm::vec2 pos;
		ElementSize width;
		ElementSize height;
		Overflow overflow;  // CURSED: assigning a default value causes undefined behaviour, memory coruption
		Color background_color;

	public:
		Text* addText();
		Wrap* addWrap();

		// addOnClickEvent(event)
		// addOnHoverEvent(function point of signature)
		// listOnHoverEvents(events with their name)
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

		uint32_t instance_start_idx;
	};

	class Text {
	public:
		// Internal
		NodeComponent node_comp;
		std::vector<CharacterDrawcall> drawcalls;

	public:
		std::u32string text;
		glm::vec2 pos;
		float size;
		float line_height;
		Color color;
	};


	enum class NodeType {
		WRAP,
		FLEX,
		TEXT
	};

	class Node {
	public:
		NodeType type;
		void* elem = nullptr;

		Node* parent;
		std::list<Node*> children;

	public:
		Wrap* createWrap();
		Flex* createFlex();
		Text* createText();

		~Node();
	};


	struct WindowCrateInfo {
		uint32_t width;
		uint32_t height;
	};

	struct AncestorProps {
		float width;
		float height;
		uint32_t clip_mask;
	};

	struct DescendantProps {
		glm::vec2* pos;
		float width;
		float height;
	};

	class Window {
	public:
		Instance* instance;

		HINSTANCE hinstance;
		WNDCLASS window_class;
		HWND hwnd;

		// Properties
		uint32_t width;
		uint32_t height;
		uint32_t surface_width;
		uint32_t surface_height;
		
		// Messages
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
		IDXGIAdapter* adapter;
		ComPtr<ID3D11Device> device;
		ComPtr<ID3D11Device5> dev5;
		ComPtr<ID3D11DeviceContext> im_ctx;
		ComPtr<ID3D11DeviceContext4> im_ctx4;
		ComPtr<IDXGISwapChain1> swapchain;

		ComPtr<ID3D11Texture2D> present_img;	
		ComPtr<ID3D11Texture2D> parents_clip_mask_img;
		ComPtr<ID3D11Texture2D> next_parents_clip_mask_img;
		ComPtr<ID3D11Texture2D> chars_atlas_tex;

		ComPtr<ID3D11SamplerState> chars_atlas_sampler;

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

		std::vector<char> wrap_vs_cso;
		std::vector<char> chars_vs_cso;

		ComPtr<ID3D11VertexShader> wrap_vs;
		ComPtr<ID3D11VertexShader> chars_vs;

		ComPtr<ID3D11RasterizerState> rasterizer_state;

		std::vector<char> wrap_ps_cso;
		std::vector<char> chars_ps_cso;

		ComPtr<ID3D11PixelShader> wrap_ps;
		ComPtr<ID3D11PixelShader> chars_ps;

		ComPtr<ID3D11BlendState> blend_state;

	public:
		void generateDrawcalls(Node* node, AncestorProps& ancestor,
			DescendantProps& r_descendant);

		void calcGlobalPositions(Node* node, glm::vec2 pos);

		ErrStack generateGPU_Data();

		// ErrStack draw();
		ErrStack draw2();

	public:
		Wrap* getRoot();
	};

	 
	extern std::list<Window> windows;


	class Instance {
	public:
		CharacterAtlas char_atlas;

	public:
		ErrStack create();

		ErrStack createWindow(WindowCrateInfo& info, Window*& r_window);

		ErrStack update();

		~Instance();
	};
}
