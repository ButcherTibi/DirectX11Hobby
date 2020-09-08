#pragma once

#include "pch.h"


#include "VulkanWrapper.hpp"
#include "GPU_Types.hpp"
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

	// wrap coordinate origin
	// Wrap self anchor


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

	public:

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
		uint32_t width = 1024;
		uint32_t height = 720;
	};

	struct AncestorProps {
		glm::vec2 pos;
		float width;
		float height;
		uint32_t clip_mask;
	};

	struct DescendantProps {
		glm::vec2 pos;
		float width;
		float height;
	};

	class Window {
	public:
		Instance* instance;

		HINSTANCE hinstance;
		HWND hwnd;

		// Properties
		uint32_t width;
		uint32_t height;
		
		// Messages
		bool minimized;
		bool close;

		// UI
		uint32_t clip_mask_id;
		std::list<Node> nodes;

		// Rendering
		bool rendering_configured;

		vkw::VulkanDevice dev;
		
		uint32_t char_instance_count;
		uint32_t wrap_instance_count;
		
		std::vector<GPU_CharacterVertex> char_verts;
		std::vector<uint32_t> char_idxs;
		std::vector<GPU_CharacterInstance> char_instances;
		GPU_CommonsUniform common_uniform;
		std::array<GPU_WrapVertex, 4> wrap_verts;
		std::array<uint32_t, 6> wrap_idxs;
		std::vector<GPU_WrapInstance> wrap_instances;

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

		vkw::CommandList cmd_list;

	public:
		void generateDrawcalls(Node* node, AncestorProps& ancestor,
			DescendantProps& r_descendant);

		ErrStack generateGPU_Data();

		//ErrStack resize();
		ErrStack draw();

	public:
		Wrap* getRoot();
	};

	 
	extern std::list<Window> windows;


	class Instance {
	public:
		vkw::Instance inst;

		CharacterAtlas char_atlas;

	public:
		ErrStack create();

		ErrStack createWindow(WindowCrateInfo& info, Window*& r_window);

		ErrStack update();

		~Instance();
	};
}
