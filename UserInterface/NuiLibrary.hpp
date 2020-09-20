#pragma once

#include "pch.h"


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


	enum class Overflow {
		OVERFLOw,
		CLIP
	};


	enum class ContentSizeType {
		RELATIVE_SIZE,
		ABSOLUTE_SIZE,
		FIT
	};

	struct ContentSize {
		ContentSizeType type = ContentSizeType::FIT;
		float size;

		void setAbsolute(float size);
		void setRelative(float size);
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

	// Wrap Ideas:
	// - self origin, child pos can refer to center of child
	// Flex

	// Dynamic:
	// - collider check
	// - redraw

	class NodeComponent {
	public:
		Window* window;

		Node* this_elem;	

	public:
		Text* addText();
		Wrap* addWrap();
		Flex* addFlex();
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
		ContentSize width;
		ContentSize height;
		Overflow overflow;  // CURSED: assigning a default value causes undefined behaviour, memory coruption
		glm::vec4 background_color;

	public:
		Text* addText();
		Wrap* addWrap();
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
		glm::vec4 color;
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

		// Vulkan
		/*vkw::VulkanDevice dev;
		vkw::Buffer chars_vbuff;
		vkw::Buffer chars_idxbuff;
		vkw::Buffer chars_instabuff;
		vkw::Buffer text_ubuff;
		vkw::Buffer wrap_vbuff;
		vkw::Buffer wrap_idxbuff;
		vkw::Buffer wrap_instabuff;

		vkw::Image composition_img;
		vkw::Image parents_clip_mask_img;
		vkw::Image next_parents_clip_mask_img;
		vkw::Image char_atlas_tex;

		vkw::ImageView composition_view;
		vkw::ImageView parents_clip_mask_view;
		vkw::ImageView next_parents_clip_mask_view;
		vkw::ImageView char_atlas_view;

		vkw::Sampler text_sampler;

		vkw::Shader text_vs;
		vkw::Shader text_fs;
		vkw::Shader wrap_vs;
		vkw::Shader wrap_fs;

		vkw::Drawpass text_pass;
		vkw::Drawpass wrap_pass;

		vkw::Framebuffer text_framebuff;
		vkw::Framebuffer wrap_framebuff;

		vkw::CommandList cmd_list;*/

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
